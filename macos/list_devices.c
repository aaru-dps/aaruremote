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

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/storage/IOMedia.h>
#include <IOKit/storage/IOStorageProtocolCharacteristics.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../aaruremote.h"
#include "macos.h"

static void TrimWhitespace(char *buffer)
{
    size_t len;

    if(!buffer) return;

    len = strlen(buffer);
    while(len > 0 &&
          (buffer[len - 1] == ' ' || buffer[len - 1] == '\r' || buffer[len - 1] == '\n' || buffer[len - 1] == '\t'))
    {
        buffer[len - 1] = 0;
        len--;
    }
}

static void SetFallbackBus(DeviceInfo *device, int32_t device_type)
{
    if(!device || device->bus[0] != 0) return;

    switch(device_type)
    {
        case AARUREMOTE_DEVICE_TYPE_ATA:
            strncpy(device->bus, "ATA", sizeof(device->bus) - 1);
            break;
        case AARUREMOTE_DEVICE_TYPE_ATAPI:
            strncpy(device->bus, "ATAPI", sizeof(device->bus) - 1);
            break;
        case AARUREMOTE_DEVICE_TYPE_SCSI:
            strncpy(device->bus, "SCSI", sizeof(device->bus) - 1);
            break;
        case AARUREMOTE_DEVICE_TYPE_SECURE_DIGITAL:
        case AARUREMOTE_DEVICE_TYPE_MMC:
            strncpy(device->bus, "MMC/SD", sizeof(device->bus) - 1);
            break;
        case AARUREMOTE_DEVICE_TYPE_NVME:
            strncpy(device->bus, "NVMe", sizeof(device->bus) - 1);
            break;
        default:
            break;
    }
}

static void NormalizeVendorAndModel(DeviceInfo *device)
{
    char  tmp_model[256];
    char *space;

    if(!device) return;

    TrimWhitespace(device->vendor);
    TrimWhitespace(device->model);
    TrimWhitespace(device->serial);
    TrimWhitespace(device->bus);

    if(device->vendor[0] == 0 && strncmp(device->bus, kIOPropertyPhysicalInterconnectTypeAppleFabric, 12) == 0)
        strncpy(device->vendor, "Apple", sizeof(device->vendor) - 1);

    if(device->vendor[0] == 0 && device->model[0] != 0)
    {
        memset(tmp_model, 0, sizeof(tmp_model));
        strncpy(tmp_model, device->model, sizeof(tmp_model) - 1);

        if(strncmp(tmp_model, "APPLE ", 6) == 0)
        {
            strncpy(device->vendor, "Apple", sizeof(device->vendor) - 1);
            memset(device->model, 0, sizeof(device->model));
            strncpy(device->model, tmp_model + 6, sizeof(device->model) - 1);
        }
        else
        {
            space = strchr(tmp_model, ' ');

            if(space)
            {
                *space = 0;
                while(space[1] == ' ') space++;

                strncpy(device->vendor, tmp_model, sizeof(device->vendor) - 1);
                memset(device->model, 0, sizeof(device->model));
                strncpy(device->model, space + 1, sizeof(device->model) - 1);
            }
        }
    }

    if(strcmp(device->vendor, "APPLE") == 0)
    {
        memset(device->vendor, 0, sizeof(device->vendor));
        strncpy(device->vendor, "Apple", sizeof(device->vendor) - 1);
    }

    TrimWhitespace(device->vendor);
    TrimWhitespace(device->model);
}

static uint8_t IsDeviceTypeSupported(int32_t device_type)
{
    switch(device_type)
    {
        case AARUREMOTE_DEVICE_TYPE_ATA:
        case AARUREMOTE_DEVICE_TYPE_ATAPI:
        case AARUREMOTE_DEVICE_TYPE_SCSI:
        case AARUREMOTE_DEVICE_TYPE_SECURE_DIGITAL:
        case AARUREMOTE_DEVICE_TYPE_MMC:
            return 1;
        case AARUREMOTE_DEVICE_TYPE_NVME:
        default:
            return 0;
    }
}

static int ShouldSkipService(io_service_t service, const char *bus)
{
    if(bus && bus[0] != 0 && strncmp(bus, kIOPropertyPhysicalInterconnectTypeVirtual, 7) == 0) return 1;

    return AaruServiceTreeContainsClass(service, "AppleAPFS");
}

DeviceInfoList *ListDevices()
{
    CFMutableDictionaryRef matching;
    io_iterator_t          iterator;
    io_service_t           service;
    DeviceInfoList        *list_start;
    DeviceInfoList        *list_current;
    DeviceInfoList        *list_next;
    kern_return_t          kr;
    char                   bsd_name[256];
    io_name_t              service_name;
    DeviceInfo             info;
    int32_t                device_type;

    list_start   = NULL;
    list_current = NULL;

    matching = IOServiceMatching(kIOMediaClass);
    if(!matching) return NULL;

    CFDictionarySetValue(matching, CFSTR(kIOMediaWholeKey), kCFBooleanTrue);

    kr = IOServiceGetMatchingServices(AARUREMOTE_IOKIT_PORT, matching, &iterator);
    if(kr != KERN_SUCCESS) return NULL;

    for(service = IOIteratorNext(iterator); service != IO_OBJECT_NULL; service = IOIteratorNext(iterator))
    {
        memset(bsd_name, 0, sizeof(bsd_name));
        memset(service_name, 0, sizeof(service_name));
        memset(&info, 0, sizeof(info));

        if(!CopyRegistryStringProperty(service, NULL, kIOBSDNameKey, bsd_name, sizeof(bsd_name)) || bsd_name[0] == 0)
        {
            IOObjectRelease(service);
            continue;
        }

        snprintf(info.path, sizeof(info.path), "/dev/r%s", bsd_name);
        PopulateDeviceProperties(service, info.vendor, sizeof(info.vendor), info.model, sizeof(info.model), info.serial,
                                 sizeof(info.serial), info.bus, sizeof(info.bus));

        if(ShouldSkipService(service, info.bus))
        {
            IOObjectRelease(service);
            continue;
        }

        device_type    = DetermineDeviceType(service, info.path);
        info.supported = IsDeviceTypeSupported(device_type);

        SetFallbackBus(&info, device_type);

        if(info.model[0] == 0 && IORegistryEntryGetName(service, service_name) == KERN_SUCCESS)
            strncpy(info.model, service_name, sizeof(info.model) - 1);

        NormalizeVendorAndModel(&info);

        list_next = malloc(sizeof(DeviceInfoList));
        if(!list_next)
        {
            IOObjectRelease(service);
            break;
        }

        memset(list_next, 0, sizeof(DeviceInfoList));
        memcpy(&list_next->this, &info, sizeof(DeviceInfo));

        if(!list_start) list_start = list_next;
        if(list_current) list_current->next = list_next;
        list_current = list_next;

        IOObjectRelease(service);
    }

    IOObjectRelease(iterator);
    return list_start;
}
