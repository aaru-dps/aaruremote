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
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>

#include "../aaruremote.h"
#include "freebsd.h"

#ifdef HAS_CAM_ATA
static u_int32_t AtaProtocolToCamFlags(uint8_t protocol)
{
    switch(protocol)
    {
        case AARUREMOTE_ATA_PROTOCOL_DEVICE_DIAGNOSTIC:
        case AARUREMOTE_ATA_PROTOCOL_DEVICE_RESET:
        case AARUREMOTE_ATA_PROTOCOL_HARD_RESET:
        case AARUREMOTE_ATA_PROTOCOL_NO_DATA:
        case AARUREMOTE_ATA_PROTOCOL_SOFT_RESET:
        case AARUREMOTE_ATA_PROTOCOL_RETURN_RESPONSE:
            return CAM_DIR_NONE;
        case AARUREMOTE_ATA_PROTOCOL_PIO_IN:
        case AARUREMOTE_ATA_PROTOCOL_UDMA_IN:
            return CAM_DIR_IN;
        case AARUREMOTE_ATA_PROTOCOL_PIO_OUT:
        case AARUREMOTE_ATA_PROTOCOL_UDMA_OUT:
            return CAM_DIR_OUT;
        default:
            return 0;
    }
}

static int32_t CamSendAtaChsCommand(void *device_ctx, AtaRegistersChs registers, AtaErrorRegistersChs *error_registers,
                                    uint8_t protocol, uint8_t transfer_register, char *buffer, uint32_t timeout,
                                    uint8_t transfer_blocks, uint32_t *duration, uint32_t *sense, uint32_t *buf_len)
{
    DeviceContext *ctx = device_ctx;
    *duration          = 0;
    *sense             = false;
    union ccb      *camccb;
    int             error;
    int             clock_error;
    struct timespec start_tp;
    struct timespec end_tp;
    double          start, end;

    if(!ctx) return -1;
    if(!ctx->device) return -1;

    camccb = cam_getccb(ctx->device);

    if(!camccb) return -1;

    camccb->ccb_h.func_code   = XPT_ATA_IO;
    camccb->ccb_h.flags       = AtaProtocolToCamFlags(protocol);
    camccb->ccb_h.retry_count = 1;
    camccb->ccb_h.cbfcnp      = NULL;
    camccb->ccb_h.timeout     = timeout;
    camccb->ataio.data_ptr    = (u_int8_t *)buffer;
    camccb->ataio.dxfer_len   = *buf_len;
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

    camccb->ataio.cmd.command      = registers.command;
    camccb->ataio.cmd.lba_high     = registers.cylinder_high;
    camccb->ataio.cmd.lba_mid      = registers.cylinder_low;
    camccb->ataio.cmd.device       = registers.device_head;
    camccb->ataio.cmd.features     = registers.feature;
    camccb->ataio.cmd.sector_count = registers.sector_count;
    camccb->ataio.cmd.lba_low      = registers.sector;

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
       (camccb->ccb_h.status & CAM_STATUS_MASK) != CAM_ATA_STATUS_ERROR)
    {
        error  = errno;
        *sense = true;
    }

    if((camccb->ccb_h.status & CAM_STATUS_MASK) == CAM_ATA_STATUS_ERROR) *sense = true;

    error_registers->cylinder_high = camccb->ataio.res.lba_high;
    error_registers->cylinder_low  = camccb->ataio.res.lba_mid;
    error_registers->device_head   = camccb->ataio.res.device;
    error_registers->error         = camccb->ataio.res.error;
    error_registers->sector        = camccb->ataio.res.lba_low;
    error_registers->sector_count  = camccb->ataio.res.sector_count;
    error_registers->status        = camccb->ataio.res.status;

    *buf_len = camccb->ataio.dxfer_len;

    cam_freeccb(camccb);

    *sense |= error_registers->error || (error_registers->status & 0xA5) || error;

    return error;
}

static int32_t CamSendAtaLba28Command(void *device_ctx, AtaRegistersLba28 registers,
                                      AtaErrorRegistersLba28 *error_registers, uint8_t protocol,
                                      uint8_t transfer_register, char *buffer, uint32_t timeout,
                                      uint8_t transfer_blocks, uint32_t *duration, uint32_t *sense, uint32_t *buf_len)
{
    DeviceContext *ctx = device_ctx;
    *duration          = 0;
    *sense             = false;
    union ccb      *camccb;
    int             error;
    int             clock_error;
    struct timespec start_tp;
    struct timespec end_tp;
    double          start, end;

    if(!ctx) return -1;
    if(!ctx->device) return -1;

    camccb = cam_getccb(ctx->device);

    if(!camccb) return -1;

    camccb->ccb_h.func_code   = XPT_ATA_IO;
    camccb->ccb_h.flags       = AtaProtocolToCamFlags(protocol);
    camccb->ccb_h.retry_count = 1;
    camccb->ccb_h.cbfcnp      = NULL;
    camccb->ccb_h.timeout     = timeout;
    camccb->ataio.data_ptr    = (u_int8_t *)buffer;
    camccb->ataio.dxfer_len   = *buf_len;
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

    camccb->ataio.cmd.command      = registers.command;
    camccb->ataio.cmd.lba_high     = registers.lba_high;
    camccb->ataio.cmd.lba_mid      = registers.lba_mid;
    camccb->ataio.cmd.device       = 0x40 | registers.device_head;
    camccb->ataio.cmd.features     = registers.feature;
    camccb->ataio.cmd.sector_count = registers.sector_count;
    camccb->ataio.cmd.lba_low      = registers.lba_low;

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
       (camccb->ccb_h.status & CAM_STATUS_MASK) != CAM_ATA_STATUS_ERROR)
    {
        error  = errno;
        *sense = true;
    }

    if((camccb->ccb_h.status & CAM_STATUS_MASK) == CAM_ATA_STATUS_ERROR) *sense = true;

    error_registers->lba_high     = camccb->ataio.res.lba_high;
    error_registers->lba_mid      = camccb->ataio.res.lba_mid;
    error_registers->device_head  = camccb->ataio.res.device;
    error_registers->error        = camccb->ataio.res.error;
    error_registers->lba_low      = camccb->ataio.res.lba_low;
    error_registers->sector_count = camccb->ataio.res.sector_count;
    error_registers->status       = camccb->ataio.res.status;

    *buf_len = camccb->ataio.dxfer_len;

    cam_freeccb(camccb);

    *sense |= error_registers->error || (error_registers->status & 0xA5) || error;

    return error;
}

static int32_t CamSendAtaLba48Command(void *device_ctx, AtaRegistersLba48 registers,
                                      AtaErrorRegistersLba48 *error_registers, uint8_t protocol,
                                      uint8_t transfer_register, char *buffer, uint32_t timeout,
                                      uint8_t transfer_blocks, uint32_t *duration, uint32_t *sense, uint32_t *buf_len)
{
    DeviceContext *ctx = device_ctx;
    *duration          = 0;
    *sense             = false;
    union ccb      *camccb;
    int             error;
    int             clock_error;
    struct timespec start_tp;
    struct timespec end_tp;
    double          start, end;

    if(!ctx) return -1;
    if(!ctx->device) return -1;

    camccb = cam_getccb(ctx->device);

    if(!camccb) return -1;

    // TODO CHECK: 48-bit ATA CAM commands can crash FreeBSD < 9.2-RELEASE

    camccb->ccb_h.func_code   = XPT_ATA_IO;
    camccb->ccb_h.flags       = AtaProtocolToCamFlags(protocol);
    camccb->ccb_h.retry_count = 1;
    camccb->ccb_h.cbfcnp      = NULL;
    camccb->ccb_h.timeout     = timeout;
    camccb->ataio.data_ptr    = (u_int8_t *)buffer;
    camccb->ataio.dxfer_len   = *buf_len;
    camccb->ccb_h.flags |= CAM_DEV_QFRZDIS;
    camccb->ataio.cmd.flags = CAM_ATAIO_NEEDRESULT | CAM_ATAIO_48BIT;

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

    camccb->ataio.cmd.command          = registers.command;
    camccb->ataio.cmd.device           = 0x40 | registers.device_head;
    camccb->ataio.cmd.lba_high_exp     = registers.lba_high_prev;
    camccb->ataio.cmd.lba_high         = registers.lba_high_cur;
    camccb->ataio.cmd.lba_mid_exp      = registers.lba_mid_prev;
    camccb->ataio.cmd.lba_mid          = registers.lba_mid_cur;
    camccb->ataio.cmd.lba_low_exp      = registers.lba_low_prev;
    camccb->ataio.cmd.lba_low          = registers.lba_low_cur;
    camccb->ataio.cmd.features_exp     = (registers.feature & 0xFF00) >> 8;
    camccb->ataio.cmd.features         = registers.feature & 0xFF;
    camccb->ataio.cmd.sector_count_exp = (registers.sector_count & 0xFF00) >> 8;
    camccb->ataio.cmd.sector_count     = registers.sector_count & 0xFF;

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
       (camccb->ccb_h.status & CAM_STATUS_MASK) != CAM_ATA_STATUS_ERROR)
    {
        error  = errno;
        *sense = true;
    }

    if((camccb->ccb_h.status & CAM_STATUS_MASK) == CAM_ATA_STATUS_ERROR) *sense = true;

    error_registers->lba_high_prev = camccb->ataio.res.lba_high_exp;
    error_registers->lba_high_cur  = camccb->ataio.res.lba_high;
    error_registers->lba_mid_prev  = camccb->ataio.res.lba_mid_exp;
    error_registers->lba_mid_cur   = camccb->ataio.res.lba_mid;
    error_registers->lba_low_prev  = camccb->ataio.res.lba_low_exp;
    error_registers->lba_low_cur   = camccb->ataio.res.lba_low;
    error_registers->sector_count  = (camccb->ataio.res.sector_count_exp << 8) + camccb->ataio.res.sector_count;
    error_registers->device_head   = camccb->ataio.res.device;
    error_registers->error         = camccb->ataio.res.error;
    error_registers->status        = camccb->ataio.res.status;

    *buf_len = camccb->ataio.dxfer_len;

    cam_freeccb(camccb);

    *sense |= error_registers->error || (error_registers->status & 0xA5) || error;

    return error;
}
#endif

#if defined(HAS_ATA_IOCTL) || defined(HAS_ATA_DEVICE_IOCTL)
#ifdef HAS_ATA_IOCTL
// Before FreeBSD 6 the request is part of a command for the ata(4) control device
typedef struct ata_cmd AtaIoctlCommand;
#define ATA_IOCTL_REQUEST(iocmd) ((iocmd).u.request)
#else
// From FreeBSD 6 the request is sent to the disk itself
typedef struct ata_ioc_request AtaIoctlCommand;
#define ATA_IOCTL_REQUEST(iocmd) (iocmd)
#endif

// Sends an LBA28 ATA command through ata(4). ata(4) does not return the status and error
// registers, only whether the command failed, and only returns the count and LBA registers for commands without
// data.
static int32_t AtaIoctlSendLba28Command(DeviceContext *ctx, AtaRegistersLba28 registers,
                                        AtaErrorRegistersLba28 *error_registers, uint8_t protocol, char *buffer,
                                        uint32_t timeout, uint32_t *duration, uint32_t *sense, uint32_t *buf_len)
{
    AtaIoctlCommand iocmd;
    int             error;
    int             clock_error;
    struct timespec start_tp;
    struct timespec end_tp;
    double          start, end;

    memset(&iocmd, 0, sizeof(AtaIoctlCommand));

#ifdef HAS_ATA_IOCTL
    iocmd.channel = ctx->ata_channel;
    iocmd.device  = ctx->ata_unit;
    iocmd.cmd     = ATAREQUEST;
#endif

    ATA_IOCTL_REQUEST(iocmd).u.ata.command = registers.command;
    ATA_IOCTL_REQUEST(iocmd).u.ata.feature = registers.feature;
    ATA_IOCTL_REQUEST(iocmd).u.ata.count   = registers.sector_count;
    ATA_IOCTL_REQUEST(iocmd).u.ata.lba = ((u_int64_t)(registers.device_head & 0x0F) << 24) |
                                         ((u_int64_t)registers.lba_high << 16) | ((u_int64_t)registers.lba_mid << 8) |
                                         registers.lba_low;
    ATA_IOCTL_REQUEST(iocmd).data      = buffer;
    ATA_IOCTL_REQUEST(iocmd).count     = buffer ? *buf_len : 0;
    ATA_IOCTL_REQUEST(iocmd).timeout   = AARUREMOTE_ATA_IOCTL_TIMEOUT(timeout);

    switch(protocol)
    {
        case AARUREMOTE_ATA_PROTOCOL_PIO_IN:
        case AARUREMOTE_ATA_PROTOCOL_UDMA_IN:
            ATA_IOCTL_REQUEST(iocmd).flags = ATA_CMD_READ;
            break;
        case AARUREMOTE_ATA_PROTOCOL_PIO_OUT:
        case AARUREMOTE_ATA_PROTOCOL_UDMA_OUT:
            ATA_IOCTL_REQUEST(iocmd).flags = ATA_CMD_WRITE;
            break;
        default:
            ATA_IOCTL_REQUEST(iocmd).flags = ATA_CMD_CONTROL;
            break;
    }

    clock_error = clock_gettime(CLOCK_REALTIME_PRECISE, &start_tp);

#ifdef HAS_ATA_IOCTL
    error = ioctl(ctx->ata_fd, IOCATA, &iocmd) < 0 ? errno : ATA_IOCTL_REQUEST(iocmd).error;
#else
    error = ioctl(ctx->ata_fd, IOCATAREQUEST, &iocmd) < 0 ? errno : ATA_IOCTL_REQUEST(iocmd).error;
#endif

    if(!clock_error) clock_error = clock_gettime(CLOCK_REALTIME_PRECISE, &end_tp);

    if(!clock_error)
    {
        start = (double)start_tp.tv_sec * 1000.0;
        start += (double)start_tp.tv_nsec / 1000000.0;
        end = (double)end_tp.tv_sec * 1000.0;
        end += (double)end_tp.tv_nsec / 1000000.0;

        *duration = (uint32_t)(end - start);
    }

    // Without the real registers, report a failed command with the status error bit
    if(error)
    {
        // ata(4) copies out its whole data buffer even when the command failed, without it having been filled
        if(ATA_IOCTL_REQUEST(iocmd).flags & ATA_CMD_READ) memset(buffer, 0, ATA_IOCTL_REQUEST(iocmd).count);

        error_registers->status = 0x01;
        *sense                  = true;
        return error;
    }

    if(ATA_IOCTL_REQUEST(iocmd).flags & ATA_CMD_CONTROL)
    {
        error_registers->sector_count = ATA_IOCTL_REQUEST(iocmd).u.ata.count;
        error_registers->lba_low      = ATA_IOCTL_REQUEST(iocmd).u.ata.lba & 0xFF;
        error_registers->lba_mid      = (ATA_IOCTL_REQUEST(iocmd).u.ata.lba >> 8) & 0xFF;
        error_registers->lba_high     = (ATA_IOCTL_REQUEST(iocmd).u.ata.lba >> 16) & 0xFF;
        error_registers->device_head =
            (registers.device_head & 0xF0) | ((ATA_IOCTL_REQUEST(iocmd).u.ata.lba >> 24) & 0x0F);
    }

    return 0;
}
#endif

int32_t SendAtaChsCommand(void *device_ctx, AtaRegistersChs registers, AtaErrorRegistersChs *error_registers,
                          uint8_t protocol, uint8_t transfer_register, char *buffer, uint32_t timeout,
                          uint8_t transfer_blocks, uint32_t *duration, uint32_t *sense, uint32_t *buf_len)
{
    DeviceContext *ctx = device_ctx;

    *duration = 0;
    *sense    = 0;

    if(!ctx) return -1;

#ifdef HAS_CAM_ATA
    if(ctx->device)
        return CamSendAtaChsCommand(device_ctx, registers, error_registers, protocol, transfer_register, buffer,
                                    timeout, transfer_blocks, duration, sense, buf_len);
#endif

    return ENOTSUP;
}

int32_t SendAtaLba28Command(void *device_ctx, AtaRegistersLba28 registers, AtaErrorRegistersLba28 *error_registers,
                            uint8_t protocol, uint8_t transfer_register, char *buffer, uint32_t timeout,
                            uint8_t transfer_blocks, uint32_t *duration, uint32_t *sense, uint32_t *buf_len)
{
    DeviceContext *ctx = device_ctx;

    *duration = 0;
    *sense    = 0;

    if(!ctx) return -1;

#ifdef HAS_CAM_ATA
    if(ctx->device)
        return CamSendAtaLba28Command(device_ctx, registers, error_registers, protocol, transfer_register, buffer,
                                      timeout, transfer_blocks, duration, sense, buf_len);
#endif

#if defined(HAS_ATA_IOCTL) || defined(HAS_ATA_DEVICE_IOCTL)
    if(ctx->ata_fd >= 0)
        return AtaIoctlSendLba28Command(ctx, registers, error_registers, protocol, buffer, timeout, duration, sense,
                                        buf_len);
#endif

    return ENOTSUP;
}

int32_t SendAtaLba48Command(void *device_ctx, AtaRegistersLba48 registers, AtaErrorRegistersLba48 *error_registers,
                            uint8_t protocol, uint8_t transfer_register, char *buffer, uint32_t timeout,
                            uint8_t transfer_blocks, uint32_t *duration, uint32_t *sense, uint32_t *buf_len)
{
    DeviceContext *ctx = device_ctx;

    *duration = 0;
    *sense    = 0;

    if(!ctx) return -1;

#ifdef HAS_CAM_ATA
    if(ctx->device)
        return CamSendAtaLba48Command(device_ctx, registers, error_registers, protocol, transfer_register, buffer,
                                      timeout, transfer_blocks, duration, sense, buf_len);
#endif

    return ENOTSUP;
}
