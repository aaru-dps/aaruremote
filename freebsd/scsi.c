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

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>

#include "../aaruremote.h"
#include "freebsd.h"

#ifdef HAS_ATA_DEVICE_IOCTL
// Sends a SCSI command to an ATAPI drive attached to ata(4) instead of CAM, which returns its sense data on errors
static int32_t AtaIoctlSendScsiCommand(DeviceContext *ctx, char *cdb, char *buffer, char **sense_buffer,
                                       uint32_t timeout, int32_t direction, uint32_t *duration, uint32_t *sense,
                                       uint32_t cdb_len, uint32_t *buf_len, uint32_t *sense_len)
{
    struct ata_ioc_request ioc;
    int                    error;
    int                    clock_error;
    struct timespec        start_tp;
    struct timespec        end_tp;
    double                 start, end;

    if(cdb_len > sizeof(ioc.u.atapi.ccb)) return ENOTSUP;

    memset(&ioc, 0, sizeof(struct ata_ioc_request));

    memcpy(ioc.u.atapi.ccb, cdb, cdb_len);
    ioc.data    = buffer;
    ioc.count   = buffer ? *buf_len : 0;
    ioc.flags   = ATA_CMD_ATAPI;
    ioc.timeout = AARUREMOTE_ATA_IOCTL_TIMEOUT(timeout);

    // ata(4) only moves data in one direction, so commands that may do both are taken as reading
    if(ioc.count > 0) ioc.flags |= direction == AARUREMOTE_SCSI_DIRECTION_OUT ? ATA_CMD_WRITE : ATA_CMD_READ;

    clock_error = clock_gettime(CLOCK_REALTIME_PRECISE, &start_tp);

    error = ioctl(ctx->ata_fd, IOCATAREQUEST, &ioc) < 0 ? errno : 0;

    if(!clock_error) clock_error = clock_gettime(CLOCK_REALTIME_PRECISE, &end_tp);

    if(!clock_error)
    {
        start = (double)start_tp.tv_sec * 1000.0;
        start += (double)start_tp.tv_nsec / 1000000.0;
        end = (double)end_tp.tv_sec * 1000.0;
        end += (double)end_tp.tv_nsec / 1000000.0;

        *duration = (uint32_t)(end - start);
    }

    if(error) return error;

    if(ioc.error)
    {
        *sense = true;

        // ata(4) copies out its whole data buffer even when the command failed, without it having been filled
        if(ioc.flags & ATA_CMD_READ) memset(buffer, 0, ioc.count);

        // ata(4) requests the sense data itself when an ATAPI command fails
        *sense_buffer = malloc(sizeof(struct atapi_sense));

        if(*sense_buffer)
        {
            memcpy(*sense_buffer, &ioc.u.atapi.sense, sizeof(struct atapi_sense));
            *sense_len = sizeof(struct atapi_sense);
        }
    }

    return ioc.error;
}
#endif

int32_t SendScsiCommand(void *device_ctx, char *cdb, char *buffer, char **sense_buffer, uint32_t timeout,
                        int32_t direction, uint32_t *duration, uint32_t *sense, uint32_t cdb_len, uint32_t *buf_len,
                        uint32_t *sense_len)
{
    DeviceContext *ctx = device_ctx;
    *sense_len         = 0;
    *sense_buffer      = NULL;
    *duration          = 0;
    *sense             = false;
    union ccb      *camccb;
    u_int32_t       flags;
    int             error;
    int             clock_error;
    struct timespec start_tp;
    struct timespec end_tp;
    double          start, end;

    switch(direction)
    {
        case AARUREMOTE_SCSI_DIRECTION_NONE:
            flags = CAM_DIR_NONE;
            break;
        case AARUREMOTE_SCSI_DIRECTION_OUT:
            flags = CAM_DIR_OUT;
            break;
        case AARUREMOTE_SCSI_DIRECTION_IN:
            flags = CAM_DIR_IN;
            break;
        case AARUREMOTE_SCSI_DIRECTION_INOUT:
        case AARUREMOTE_SCSI_DIRECTION_UNSPECIFIED:
        default:
            flags = AARUREMOTE_CAM_DIR_BOTH;
            break;
    }

    if(!ctx) return -1;

#ifdef HAS_ATA_DEVICE_IOCTL
    if(ctx->ata_atapi)
        return AtaIoctlSendScsiCommand(ctx, cdb, buffer, sense_buffer, timeout, direction, duration, sense, cdb_len,
                                       buf_len, sense_len);
#endif

    if(!ctx->device) return -1;

    *sense_buffer = malloc(32);

    if(!*sense_buffer) return -1;

    camccb = cam_getccb(ctx->device);

    if(!camccb)
    {
        free(*sense_buffer);
        *sense_buffer = NULL;
        return -1;
    }

    camccb->ccb_h.func_code   = XPT_SCSI_IO;
    camccb->ccb_h.flags       = flags;
    camccb->ccb_h.retry_count = 1;
    camccb->ccb_h.cbfcnp      = NULL;
    camccb->ccb_h.timeout     = timeout;
    camccb->csio.data_ptr     = (u_int8_t *)buffer;
    camccb->csio.dxfer_len    = *buf_len;
    camccb->csio.sense_len    = 32;
    camccb->csio.cdb_len      = cdb_len;
    camccb->csio.tag_action   = 0x20;

    if(cdb_len <= CAM_MAX_CDBLEN)
        memcpy(camccb->csio.cdb_io.cdb_bytes, cdb, cdb_len);
    else
    {
        camccb->csio.cdb_io.cdb_ptr = (u_int8_t *)cdb;
        camccb->ccb_h.flags |= CAM_CDB_POINTER;
    }

    camccb->ccb_h.flags |= CAM_DEV_QFRZDIS;

    clock_error = clock_gettime(CLOCK_REALTIME_PRECISE, &start_tp);

    error = cam_send_ccb(ctx->device, camccb);

    if(!clock_error) clock_error = clock_gettime(CLOCK_REALTIME_PRECISE, &end_tp);

    if(!clock_error)
    {
        start = (double)start_tp.tv_sec * 1000.0;
        start += (double)start_tp.tv_nsec / 1000000.0;
        end = (double)end_tp.tv_sec * 1000.0;
        end += (double)end_tp.tv_nsec / 1000000.0;

        *duration = (uint32_t)(end - start);
    }

    if(error < 0) error = errno;

    if((camccb->ccb_h.status & CAM_STATUS_MASK) != CAM_REQ_CMP &&
       (camccb->ccb_h.status & CAM_STATUS_MASK) != CAM_SCSI_STATUS_ERROR)
    {
        error  = errno;
        *sense = true;
    }

    // The sense buffer holds 32 bytes, as many as were requested from CAM
    if((camccb->ccb_h.status & CAM_STATUS_MASK) == CAM_SCSI_STATUS_ERROR)
    {
        *sense             = true;
        (*sense_buffer)[0] = camccb->csio.scsi_status;
        *sense_len         = 1;
    }

    if((camccb->ccb_h.status & CAM_AUTOSNS_VALID) && camccb->csio.sense_len - camccb->csio.sense_resid > 0)
    {
        *sense             = (camccb->ccb_h.status & CAM_STATUS_MASK) == CAM_SCSI_STATUS_ERROR;
        *sense_len         = camccb->csio.sense_len - camccb->csio.sense_resid;

        // Copied as raw bytes, as the layout of struct scsi_sense_data differs between FreeBSD releases
        memcpy(*sense_buffer, &camccb->csio.sense_data, *sense_len);
    }

    *buf_len = camccb->csio.dxfer_len;

    cam_freeccb(camccb);

    return error;
}
