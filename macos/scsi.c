/*
 * This file is part of the Aaru Remote Server.
 * Copyright (c) 2019-2026 Natalia Portillo.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <CoreFoundation/CFPlugInCOM.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include <IOKit/IOCFPlugIn.h>
#include <IOKit/scsi/SCSICmds_REQUEST_SENSE_Defs.h>
#include <IOKit/scsi/SCSITaskLib.h>

#include "../aaruremote.h"
#include "macos.h"

static uint32_t AaruGetMilliseconds()
{
    struct timeval tv;

    if(gettimeofday(&tv, NULL) != 0) return 0;

    return (uint32_t)(tv.tv_sec * 1000U + tv.tv_usec / 1000U);
}

static UInt8 AaruScsiDirection(int32_t direction)
{
    switch(direction)
    {
        case AARUREMOTE_SCSI_DIRECTION_IN:
            return kSCSIDataTransfer_FromTargetToInitiator;
        case AARUREMOTE_SCSI_DIRECTION_OUT:
            return kSCSIDataTransfer_FromInitiatorToTarget;
        case AARUREMOTE_SCSI_DIRECTION_NONE:
            return kSCSIDataTransfer_NoDataTransfer;
        case AARUREMOTE_SCSI_DIRECTION_INOUT:
        case AARUREMOTE_SCSI_DIRECTION_UNSPECIFIED:
        default:
            return kSCSIDataTransfer_FromTargetToInitiator;
    }
}

static int32_t AaruMapIOReturnToErrno(IOReturn io_return)
{
    switch(io_return)
    {
        case kIOReturnSuccess:
            return 0;
        case kIOReturnNoDevice:
        case kIOReturnNotFound:
            return ENODEV;
        case kIOReturnNoMemory:
        case kIOReturnNoResources:
        case kIOReturnVMError:
            return ENOMEM;
        case kIOReturnBusy:
        case kIOReturnCannotLock:
        case kIOReturnExclusiveAccess:
            return EBUSY;
        case kIOReturnNotOpen:
        case kIOReturnNotPrivileged:
            return EACCES;
        case kIOReturnAborted:
            return ECANCELED;
        case kIOReturnNoSpace:
            return ENOSPC;
        case kIOReturnBadArgument:
            return EINVAL;
        case kIOReturnUnsupported:
            return ENOTSUP;
        case kIOReturnTimeout:
            return ETIMEDOUT;
        default:
            return EIO;
    }
}

static int32_t AaruMapTaskStatusToErrno(SCSITaskStatus task_status)
{
    switch(task_status)
    {
        case kSCSITaskStatus_GOOD:
        case kSCSITaskStatus_CHECK_CONDITION:
        case kSCSITaskStatus_CONDITION_MET:
        case kSCSITaskStatus_INTERMEDIATE:
        case kSCSITaskStatus_INTERMEDIATE_CONDITION_MET:
            return 0;
        case kSCSITaskStatus_TaskTimeoutOccurred:
            return ETIMEDOUT;
        case kSCSITaskStatus_BUSY:
        case kSCSITaskStatus_TASK_SET_FULL:
            return EBUSY;
        case kSCSITaskStatus_RESERVATION_CONFLICT:
        case kSCSITaskStatus_ACA_ACTIVE:
            return EACCES;
        case kSCSITaskStatus_DeliveryFailure:
        case kSCSITaskStatus_No_Status:
            return EIO;
        default:
            return 0;
    }
}

static IOReturn CreateTaskDeviceInterface(io_service_t service, SCSITaskDeviceInterface ***task_device_interface)
{
    io_registry_entry_t   current;
    io_registry_entry_t   parent;
    IOCFPlugInInterface **plugin_interface;
    SInt32                score;
    HRESULT               hresult;
    IOReturn              io_return;

    if(!task_device_interface) return kIOReturnBadArgument;

    *task_device_interface = NULL;
    if(service == IO_OBJECT_NULL) return kIOReturnNoDevice;

    current = service;

    while(current != IO_OBJECT_NULL)
    {
        plugin_interface = NULL;
        score            = 0;
        io_return        = IOCreatePlugInInterfaceForService(current, kIOSCSITaskDeviceUserClientTypeID,
                                                             kIOCFPlugInInterfaceID, &plugin_interface, &score);
        if(io_return == kIOReturnSuccess && plugin_interface != NULL)
        {
            hresult = (*plugin_interface)
                          ->QueryInterface(plugin_interface, CFUUIDGetUUIDBytes(kIOSCSITaskDeviceInterfaceID),
                                           (LPVOID *)task_device_interface);
            (*plugin_interface)->Release(plugin_interface);

            if(hresult == S_OK && *task_device_interface != NULL)
            {
                if(current != service) IOObjectRelease(current);
                return kIOReturnSuccess;
            }
        }

        parent = IO_OBJECT_NULL;
        if(IORegistryEntryGetParentEntry(current, kIOServicePlane, &parent) != KERN_SUCCESS) break;

        if(current != service) IOObjectRelease(current);
        current = parent;
    }

    if(current != IO_OBJECT_NULL && current != service) IOObjectRelease(current);

    return kIOReturnNotFound;
}

int32_t SendScsiCommand(void *device_ctx, char *cdb, char *buffer, char **sense_buffer, uint32_t timeout,
                        int32_t direction, uint32_t *duration, uint32_t *sense, uint32_t cdb_len, uint32_t *buf_len,
                        uint32_t *sense_len)
{
    DeviceContext            *ctx;
    io_service_t              service;
    SCSITaskDeviceInterface **task_device_interface;
    SCSITaskInterface       **task_interface;
    SCSI_Sense_Data           sense_data;
    SCSITaskStatus            task_status;
    SCSITaskSGElement         sg_element;
    UInt64                    transferred;
    UInt8                     scsi_direction;
    IOReturn                  io_return;
    uint32_t                  start_ms;
    uint32_t                  end_ms;
    int32_t                   error;

    ctx = device_ctx;
    if(duration) *duration = 0;
    if(sense) *sense = 0;
    if(sense_len) *sense_len = 0;

    if(!sense_buffer || !ctx) return -1;

    *sense_buffer = NULL;

    if(cdb_len == 0 || cdb_len > 16) return ENOTSUP;

    service = FindDeviceServiceByPath(ctx->device_path);
    if(service == IO_OBJECT_NULL) return ENODEV;

    task_device_interface = NULL;
    io_return             = CreateTaskDeviceInterface(service, &task_device_interface);
    IOObjectRelease(service);

    if(io_return != kIOReturnSuccess || task_device_interface == NULL) return AaruMapIOReturnToErrno(io_return);

    task_interface = (*task_device_interface)->CreateSCSITask(task_device_interface);
    if(task_interface == NULL)
    {
        (*task_device_interface)->Release(task_device_interface);
        return ENOMEM;
    }

    io_return = (*task_device_interface)->ObtainExclusiveAccess(task_device_interface);
    if(io_return != kIOReturnSuccess)
    {
        (*task_interface)->Release(task_interface);
        (*task_device_interface)->Release(task_device_interface);
        return AaruMapIOReturnToErrno(io_return);
    }

    memset(&sense_data, 0, sizeof(sense_data));
    transferred    = 0;
    scsi_direction = AaruScsiDirection(direction);

    io_return = (*task_interface)->SetCommandDescriptorBlock(task_interface, (UInt8 *)cdb, (UInt8)cdb_len);
    if(io_return != kIOReturnSuccess)
    {
        (*task_interface)->Release(task_interface);
        (*task_device_interface)->ReleaseExclusiveAccess(task_device_interface);
        (*task_device_interface)->Release(task_device_interface);
        return AaruMapIOReturnToErrno(io_return);
    }

    if(timeout > 0) (*task_interface)->SetTimeoutDuration(task_interface, timeout);

    if(buffer != NULL && buf_len != NULL && *buf_len > 0 && scsi_direction != kSCSIDataTransfer_NoDataTransfer)
    {
        memset(&sg_element, 0, sizeof(sg_element));
        sg_element.address = (mach_vm_address_t)buffer;
        sg_element.length  = *buf_len;
        io_return =
            (*task_interface)->SetScatterGatherEntries(task_interface, &sg_element, 1, *buf_len, scsi_direction);
        if(io_return != kIOReturnSuccess)
        {
            (*task_interface)->Release(task_interface);
            (*task_device_interface)->ReleaseExclusiveAccess(task_device_interface);
            (*task_device_interface)->Release(task_device_interface);
            return AaruMapIOReturnToErrno(io_return);
        }
    }

    start_ms  = AaruGetMilliseconds();
    io_return = (*task_interface)->ExecuteTaskSync(task_interface, &sense_data, &task_status, &transferred);
    end_ms    = AaruGetMilliseconds();

    if(duration && end_ms >= start_ms) *duration = end_ms - start_ms;
    if(buf_len) *buf_len = (uint32_t)transferred;

    if(sense_buffer)
    {
        *sense_buffer = malloc(sizeof(SCSI_Sense_Data));
        if(*sense_buffer)
        {
            memcpy(*sense_buffer, &sense_data, sizeof(SCSI_Sense_Data));
            if(sense_len) *sense_len = sizeof(SCSI_Sense_Data);
        }
    }

    if(sense)
        *sense = (io_return != kIOReturnSuccess) || (task_status != kSCSITaskStatus_GOOD) ||
                 ((sense_data.SENSE_KEY & kSENSE_KEY_Mask) != kSENSE_KEY_NO_SENSE) ||
                 (sense_data.ADDITIONAL_SENSE_CODE != 0) || (sense_data.ADDITIONAL_SENSE_CODE_QUALIFIER != 0);

    (*task_interface)->Release(task_interface);
    (*task_device_interface)->ReleaseExclusiveAccess(task_device_interface);
    (*task_device_interface)->Release(task_device_interface);

    error = AaruMapIOReturnToErrno(io_return);
    if(error == 0) error = AaruMapTaskStatusToErrno(task_status);

    return error;
}
