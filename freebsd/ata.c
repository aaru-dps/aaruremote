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
#include <time.h>

#include "../aaruremote.h"
#include "freebsd.h"

static u_int32_t AtaProtocolToCamFlags(uint8_t protocol)
{
    switch(protocol)
    {
        case AARUREMOTE_ATA_PROTOCOL_DEVICE_DIAGNOSTIC:
        case AARUREMOTE_ATA_PROTOCOL_DEVICE_RESET:
        case AARUREMOTE_ATA_PROTOCOL_HARD_RESET:
        case AARUREMOTE_ATA_PROTOCOL_NO_DATA:
        case AARUREMOTE_ATA_PROTOCOL_SOFT_RESET:
        case AARUREMOTE_ATA_PROTOCOL_RETURN_RESPONSE: return CAM_DIR_NONE;
        case AARUREMOTE_ATA_PROTOCOL_PIO_IN:
        case AARUREMOTE_ATA_PROTOCOL_UDMA_IN: return CAM_DIR_IN;
        case AARUREMOTE_ATA_PROTOCOL_PIO_OUT:
        case AARUREMOTE_ATA_PROTOCOL_UDMA_OUT: return CAM_DIR_OUT;
        default: return 0;
    }
}

int32_t SendAtaChsCommand(void*                 device_ctx,
                          AtaRegistersChs       registers,
                          AtaErrorRegistersChs* error_registers,
                          uint8_t               protocol,
                          uint8_t               transfer_register,
                          char*                 buffer,
                          uint32_t              timeout,
                          uint8_t               transfer_blocks,
                          uint32_t*             duration,
                          uint32_t*             sense,
                          uint32_t*             buf_len)
{
    DeviceContext* ctx = device_ctx;
    *duration = 0;
    *sense    = false;
    union ccb* camccb;
    int error;
    int clock_error;
    struct timespec start_tp;
    struct timespec end_tp;
    double start, end;

    if(!ctx) return -1;
    if(!ctx->device) return -1;

    camccb = cam_getccb(ctx->device);

    if(!camccb) return -1;

    camccb->ccb_h.func_code = XPT_ATA_IO;
    camccb->ccb_h.flags = AtaProtocolToCamFlags(protocol);
    camccb->ccb_h.xflags = 0;
    camccb->ccb_h.retry_count = 1;
    camccb->ccb_h.cbfcnp = NULL;
    camccb->ccb_h.timeout = timeout;
    camccb->ataio.data_ptr = (u_int8_t *)buffer;
    camccb->ataio.dxfer_len = *buf_len;
    camccb->ccb_h.flags |= CAM_DEV_QFRZDIS;
    camccb->ataio.cmd.flags = CAM_ATAIO_NEEDRESULT;

    switch(protocol)
    {
        case AARUREMOTE_ATA_PROTOCOL_DMA:
        case AARUREMOTE_ATA_PROTOCOL_DMA_QUEUED:
        case AARUREMOTE_ATA_PROTOCOL_UDMA_IN:
        case AARUREMOTE_ATA_PROTOCOL_UDMA_OUT:
            camccb->ataio.cmd.flags |= CAM_ATAIO_DMA;
            break;
        case AARUREMOTE_ATA_PROTOCOL_FPDMA:
            camccb->ataio.cmd.flags |= CAM_ATAIO_FPDMA;
            break;
    }

    camccb->ataio.cmd.command = registers.command;
    camccb->ataio.cmd.lba_high = registers.cylinder_high;
    camccb->ataio.cmd.lba_mid =registers.cylinder_low;
    camccb->ataio.cmd.device = 0x40 |registers.device_head;
    camccb->ataio.cmd.features = registers.feature;
    camccb->ataio.cmd.sector_count= registers.sector_count;
    camccb->ataio.cmd.lba_low= registers.sector;

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
        (camccb->ccb_h.status & CAM_STATUS_MASK) != CAM_ATA_STATUS_ERROR)
    {
        error = errno;
        *sense = true;
    }

    if((camccb->ccb_h.status & CAM_STATUS_MASK) == CAM_ATA_STATUS_ERROR)
        *sense = true;

    error_registers->cylinder_high = camccb->ataio.res.lba_high;
    error_registers->cylinder_low  = camccb->ataio.res.lba_mid;
    error_registers->device_head   = camccb->ataio.res.device;
    error_registers->error        = camccb->ataio.res.error;
    error_registers->sector       = camccb->ataio.res.lba_low;
    error_registers->sector_count  = camccb->ataio.res.sector_count;
    error_registers->status       = camccb->ataio.res.status;

    cam_freeccb(camccb);

    *buf_len = camccb->ataio.dxfer_len;

    *sense |= error_registers->error || (error_registers->status & 0xA5) || error;

    return error;
}

int32_t SendAtaLba28Command(void*                   device_ctx,
                            AtaRegistersLba28       registers,
                            AtaErrorRegistersLba28* error_registers,
                            uint8_t                 protocol,
                            uint8_t                 transfer_register,
                            char*                   buffer,
                            uint32_t                timeout,
                            uint8_t                 transfer_blocks,
                            uint32_t*               duration,
                            uint32_t*               sense,
                            uint32_t*               buf_len)
{
    return -1;
}

int32_t SendAtaLba48Command(void*                   device_ctx,
                            AtaRegistersLba48       registers,
                            AtaErrorRegistersLba48* error_registers,
                            uint8_t                 protocol,
                            uint8_t                 transfer_register,
                            char*                   buffer,
                            uint32_t                timeout,
                            uint8_t                 transfer_blocks,
                            uint32_t*               duration,
                            uint32_t*               sense,
                            uint32_t*               buf_len)
{
    return -1;
}