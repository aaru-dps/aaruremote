/*
 * This file is part of the Aaru Remote Server.
 * Copyright (c) 2019-2020 Natalia Portillo.
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
#include <time.h>

#include "../aaruremote.h"
#include "freebsd.h"

int32_t SendScsiCommand(void*     device_ctx,
                        char*     cdb,
                        char*     buffer,
                        char**    sense_buffer,
                        uint32_t  timeout,
                        int32_t   direction,
                        uint32_t* duration,
                        uint32_t* sense,
                        uint32_t  cdb_len,
                        uint32_t* buf_len,
                        uint32_t* sense_len)
{
    DeviceContext* ctx = device_ctx;
    *sense_len = 0;
    *sense_buffer = NULL;
    *duration = 0;
    union ccb* camccb;
    u_int32_t flags;
    int error;
    int clock_error;
    struct timespec start_tp;
    struct timespec end_tp;
    double start, end;

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
            flags = CAM_DIR_BOTH;
            break;
    }

    if(!ctx) return -1;
    if(!ctx->device) return -1;

    *sense_buffer = malloc(32);

    if(!*sense_buffer) return -1;

    camccb = cam_getccb(ctx->device);

    if(!camccb) return -1;

    camccb->ccb_h.func_code = XPT_SCSI_IO;
    camccb->ccb_h.flags = flags;
    camccb->ccb_h.xflags = 0;
    camccb->ccb_h.retry_count = 1;
    camccb->ccb_h.cbfcnp = NULL;
    camccb->ccb_h.timeout = timeout;
    camccb->csio.data_ptr = (u_int8_t *)buffer;
    camccb->csio.dxfer_len = *buf_len;
    camccb->csio.sense_len = 32;
    camccb->csio.cdb_len = cdb_len;
    camccb->csio.tag_action = 0x20;

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

    if(!clock_error)
        clock_error = clock_gettime(CLOCK_REALTIME_PRECISE, &end_tp);

    if(!clock_error)
    {
        start = (double)start_tp.tv_sec * 1000.0;
        start += (double)start_tp.tv_nsec / 1000000.0;
        end = (double)end_tp.tv_sec * 1000.0;
        end += (double)end_tp.tv_nsec / 1000000.0;

        *duration = (uint32_t)(end-start);
    }

    if(error < 0)
        error = errno;

    if((camccb->ccb_h.status & CAM_STATUS_MASK) != CAM_REQ_CMP &&
        (camccb->ccb_h.status & CAM_STATUS_MASK) != CAM_SCSI_STATUS_ERROR)
    {
        error = errno;
        *sense = true;
    }

    if((camccb->ccb_h.status & CAM_STATUS_MASK) == CAM_SCSI_STATUS_ERROR)
    {
        *sense = true;
        *sense_buffer = malloc(1);
        (*sense_buffer)[0] = camccb->csio.scsi_status;
    }

    if((camccb->ccb_h.status & CAM_AUTOSNS_VALID) && camccb->csio.sense_len - camccb->csio.sense_resid > 0)
    {
        *sense          = (camccb->ccb_h.status & CAM_STATUS_MASK) == CAM_SCSI_STATUS_ERROR;
        *sense_buffer    = malloc(camccb->csio.sense_len - camccb->csio.sense_resid);
        (*sense_buffer)[0] = camccb->csio.sense_data.error_code;
        memcpy((*sense_buffer)+1, camccb->csio.sense_data.sense_buf, (camccb->csio.sense_len - camccb->csio.sense_resid) - 1);
    }

    cam_freeccb(camccb);

    *buf_len = camccb->csio.dxfer_len;

    return error;
}