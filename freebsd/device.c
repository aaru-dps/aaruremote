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
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "../aaruremote.h"
#include "freebsd.h"

#ifdef HAS_ATA_IOCTL
// Finds the ata(4) channel and unit of an ATA disk by its device name, such as ad0
static int FindAtaDevice(DeviceContext *ctx)
{
    struct ata_cmd iocmd;
    const char    *name;
    int            channel, max_channel, unit;

    name = strrchr(ctx->device_path, '/');
    name = name ? name + 1 : ctx->device_path;

    memset(&iocmd, 0, sizeof(struct ata_cmd));
    iocmd.cmd = ATAGMAXCHANNEL;

    if(ioctl(ctx->ata_fd, IOCATA, &iocmd) < 0) return 0;

    max_channel = iocmd.u.maxchan;

    for(channel = 0; channel < max_channel; channel++)
    {
        memset(&iocmd, 0, sizeof(struct ata_cmd));
        iocmd.channel = channel;
        iocmd.cmd     = ATAGPARM;

        if(ioctl(ctx->ata_fd, IOCATA, &iocmd) < 0) continue;

        for(unit = 0; unit < 2; unit++)
        {
            // ATAPI devices are only supported through atapicam
            if(!iocmd.u.param.type[unit] || iocmd.u.param.params[unit].config & ATA_PROTO_ATAPI_12) continue;

            if(strncmp(iocmd.u.param.name[unit], name, sizeof(iocmd.u.param.name[unit])) != 0) continue;

            ctx->ata_channel = channel;
            ctx->ata_unit    = unit;

            return 1;
        }
    }

    return 0;
}

// Opens an ATA disk attached to ata(4) instead of CAM
static int OpenAtaDevice(DeviceContext *ctx)
{
    ctx->ata_fd = open("/dev/ata", O_RDWR);

    if(ctx->ata_fd < 0) return 0;

    if(FindAtaDevice(ctx)) ctx->fd = open(ctx->device_path, O_RDONLY);

    if(ctx->fd >= 0) return 1;

    close(ctx->ata_fd);
    ctx->ata_fd = -1;

    return 0;
}
#endif

#ifdef HAS_ATA_DEVICE_IOCTL
// Opens an ATA disk attached to ata(4) instead of CAM, which takes the raw ATA commands itself
static int OpenAtaDevice(DeviceContext *ctx)
{
    struct ata_params params;

    ctx->fd = open(ctx->device_path, O_RDWR);

    if(ctx->fd < 0) return 0;

    // ATAPI devices are only supported through atapicam
    if(ioctl(ctx->fd, IOCATAGPARM, &params) == 0 && !(params.config & ATA_PROTO_ATAPI_12))
    {
        ctx->ata_fd = ctx->fd;
        return 1;
    }

    close(ctx->fd);
    ctx->fd = -1;

    return 0;
}
#endif

// Closes whichever of the CAM and ata(4) handles are open
static void CloseDevice(DeviceContext *ctx)
{
    if(ctx->device) cam_close_device(ctx->device);
    if(ctx->ata_fd >= 0 && ctx->ata_fd != ctx->fd) close(ctx->ata_fd);
    if(ctx->fd >= 0) close(ctx->fd);

    ctx->device = NULL;
    ctx->ata_fd = -1;
    ctx->fd     = -1;
}

// Opens the device through CAM, or through ata(4) for ATA disks that are not attached to CAM
static int OpenDevice(DeviceContext *ctx)
{
    ctx->ata_fd = -1;
    ctx->fd     = -1;
    ctx->device = cam_open_device(ctx->device_path, O_RDWR);

    if(ctx->device) return 1;

#if defined(HAS_ATA_IOCTL) || defined(HAS_ATA_DEVICE_IOCTL)
    if(OpenAtaDevice(ctx)) return 1;
#endif

    return 0;
}

void *DeviceOpen(const char *device_path)
{
    DeviceContext *ctx;

    ctx = malloc(sizeof(DeviceContext));

    if(!ctx) return NULL;

    memset(ctx, 0, sizeof(DeviceContext));

    strncpy(ctx->device_path, device_path, 4096);

    if(!OpenDevice(ctx))
    {
        free(ctx);
        return NULL;
    }

    return ctx;
}

void DeviceClose(void *device_ctx)
{
    DeviceContext *ctx = device_ctx;

    if(!ctx) return;

    CloseDevice(ctx);

    free(ctx);
}

int32_t GetDeviceType(void *device_ctx)
{
    DeviceContext *ctx = device_ctx;
#ifdef HAS_CAM_ATA
    union ccb *camccb;
    int        ret;
    int32_t    device_type = AARUREMOTE_DEVICE_TYPE_UNKNOWN;
#endif

    if(!ctx) return -1;

    // Only ATA disks are opened through ata(4)
    if(ctx->ata_fd >= 0) return AARUREMOTE_DEVICE_TYPE_ATA;

    if(!ctx->device) return -1;

#ifndef HAS_CAM_ATA
    // Before FreeBSD 8 CAM has no protocol information, and ATAPI devices can only be attached through atapicam
    if(strncmp(ctx->device->sim_name, "ata", 3) == 0) return AARUREMOTE_DEVICE_TYPE_ATAPI;

    return AARUREMOTE_DEVICE_TYPE_SCSI;
#else
    camccb = cam_getccb(ctx->device);

    if(!camccb) return device_type;

    camccb->ccb_h.func_code = XPT_GDEV_TYPE;

    ret = cam_send_ccb(ctx->device, camccb);

    if(ret < 0)
    {
        cam_freeccb(camccb);
        return device_type;
    }

    switch(camccb->cgd.protocol)
    {
        case PROTO_ATA:
        case PROTO_SATAPM:
            device_type = AARUREMOTE_DEVICE_TYPE_ATA;
            break;
        case PROTO_ATAPI:
            device_type = AARUREMOTE_DEVICE_TYPE_ATAPI;
            break;
        case PROTO_SCSI:
            device_type = AARUREMOTE_DEVICE_TYPE_SCSI;
            break;
#if __FreeBSD_version >= 1200000
        case PROTO_NVME:
            device_type = AARUREMOTE_DEVICE_TYPE_NVME;
            break;
        case PROTO_MMCSD:
            // TODO: MMC vs SD
            device_type = AARUREMOTE_DEVICE_TYPE_MMC;
            break;
#endif
        default:
            device_type = AARUREMOTE_DEVICE_TYPE_UNKNOWN;
            break;
    }

    cam_freeccb(camccb);
    return device_type;
#endif
}

int32_t ReOpen(void *device_ctx, uint32_t *closeFailed)
{
    DeviceContext *ctx = device_ctx;
    int            ret;
    *closeFailed = 0;

    if(!ctx) return -1;

    CloseDevice(ctx);

    return OpenDevice(ctx) ? 0 : errno;
}

int32_t OsRead(void *device_ctx, char *buffer, uint64_t offset, uint32_t length, uint32_t *duration)
{
    DeviceContext *ctx = device_ctx;
    ssize_t        ret;
    *duration = 0;
    off_t pos;
    int   fd;

    if(!ctx) return -1;

    // ata(4) disks are read through the disk itself, CAM devices through their pass device
    if(ctx->fd >= 0)
        fd = ctx->fd;
    else if(ctx->device)
        fd = ctx->device->fd;
    else
        return -1;

    // TODO: Timing
    pos = lseek(fd, (off_t)offset, SEEK_SET);

    if(pos < 0) return errno;

    // TODO: Timing
    ret = read(fd, (void *)buffer, (size_t)length);

    return ret < 0 ? errno : 0;
}
