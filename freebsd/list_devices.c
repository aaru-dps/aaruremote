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

#include <dirent.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "../aaruremote.h"
#include "freebsd.h"

// Fills the device information of SCSI devices, and of ATAPI devices from their SCSI inquiry data
static void FillScsiDeviceInfo(DeviceInfo *info, union ccb *camccb, struct cam_device *camdev)
{
    strncpy(info->vendor, camccb->cgd.inq_data.vendor, 8);
    strncpy(info->model, camccb->cgd.inq_data.product, 16);

    if(strncmp(camdev->sim_name, "ata", 3) == 0 || strncmp(camdev->sim_name, "ahcich", 6) == 0)
        strncpy(info->bus, "ATAPI", 5);
    else
        strncpy(info->bus, "SCSI", 4);

    info->supported = true;
}

#if defined(HAS_ATA_IOCTL) || defined(HAS_ATA_DEVICE_IOCTL)
// Copies a string from ATA identify data, which is padded with spaces, without the padding
static void CopyAtaString(char *dest, const u_int8_t *src, int len)
{
    strncpy(dest, (const char *)src, len);

    while(len > 0 && (dest[len - 1] == ' ' || dest[len - 1] == 0)) dest[--len] = 0;
}

// Appends an ATA disk attached to ata(4) to the device list, returning the new last entry
static DeviceInfoList *AppendAtaDevice(DeviceInfoList **list_start, DeviceInfoList *list_current, const char *name,
                                       struct ata_params *params)
{
    DeviceInfoList *list_next;

    list_next = malloc(sizeof(DeviceInfoList));

    if(!list_next) return list_current;

    memset(list_next, 0, sizeof(DeviceInfoList));

    snprintf(list_next->this.path, 1024, "/dev/%s", name);
    strncpy(list_next->this.vendor, "ATA", 3);
    CopyAtaString(list_next->this.model, params->model, 40);
    CopyAtaString(list_next->this.serial, params->serial, 20);
    strncpy(list_next->this.bus, "ATA", 4);
    list_next->this.supported = true;

    if(!*list_start) *list_start = list_next;

    if(list_current) list_current->next = list_next;

    return list_next;
}
#endif

#ifdef HAS_ATA_IOCTL

// Appends the ATA disks attached to ata(4) instead of CAM to the device list, returning the new last entry
static DeviceInfoList *ListAtaDevices(DeviceInfoList **list_start, DeviceInfoList *list_current)
{
    struct ata_cmd iocmd;
    int            ata_fd, channel, max_channel, unit;

    ata_fd = open("/dev/ata", O_RDWR);

    if(ata_fd < 0) return list_current;

    memset(&iocmd, 0, sizeof(struct ata_cmd));
    iocmd.cmd = ATAGMAXCHANNEL;

    max_channel = ioctl(ata_fd, IOCATA, &iocmd) < 0 ? 0 : iocmd.u.maxchan;

    for(channel = 0; channel < max_channel; channel++)
    {
        memset(&iocmd, 0, sizeof(struct ata_cmd));
        iocmd.channel = channel;
        iocmd.cmd     = ATAGPARM;

        if(ioctl(ata_fd, IOCATA, &iocmd) < 0) continue;

        for(unit = 0; unit < 2; unit++)
        {
            // ATAPI devices are only supported through atapicam
            if(!iocmd.u.param.type[unit] || iocmd.u.param.params[unit].config & ATA_PROTO_ATAPI_12) continue;

            list_current =
                AppendAtaDevice(list_start, list_current, iocmd.u.param.name[unit], &iocmd.u.param.params[unit]);
        }
    }

    close(ata_fd);

    return list_current;
}
#endif

#ifdef HAS_ATA_DEVICE_IOCTL
// Appends the ATA disks attached to ata(4) instead of CAM to the device list, returning the new last entry
static DeviceInfoList *ListAtaDevices(DeviceInfoList **list_start, DeviceInfoList *list_current)
{
    struct ata_ioc_devices devices;
    int                    ata_fd, channel, max_channel, unit;

    ata_fd = open("/dev/ata", O_RDWR);

    if(ata_fd < 0) return list_current;

    if(ioctl(ata_fd, IOCATAGMAXCHANNEL, &max_channel) < 0) max_channel = 0;

    for(channel = 0; channel < max_channel; channel++)
    {
        memset(&devices, 0, sizeof(struct ata_ioc_devices));
        devices.channel = channel;

        if(ioctl(ata_fd, IOCATADEVICES, &devices) < 0) continue;

        for(unit = 0; unit < 2; unit++)
        {
            // ATAPI devices are only supported through atapicam
            if(!devices.name[unit][0] || devices.params[unit].config & ATA_PROTO_ATAPI_12) continue;

            list_current = AppendAtaDevice(list_start, list_current, devices.name[unit], &devices.params[unit]);
        }
    }

    close(ata_fd);

    return list_current;
}
#endif

DeviceInfoList *ListDevices()
{
    DeviceInfoList    *list_start = NULL, *list_current = NULL, *list_next = NULL;
    DIR               *dir;
    struct dirent     *dirent;
    struct cam_device *camdev;
    union ccb         *camccb;
    int                ret;
    int                i;

    dir = opendir("/dev");
    if(!dir) return NULL;

    dirent = readdir(dir);

    while(dirent)
    {
        if(strncmp(dirent->d_name, "pass", 4) != 0)
        {
            dirent = readdir(dir);
            continue;
        }

        list_next = malloc(sizeof(DeviceInfoList));

        if(!list_next) break;

        memset(list_next, 0, sizeof(DeviceInfoList));

        snprintf(list_next->this.path, 1024, "/dev/%s", dirent->d_name);

        camdev = cam_open_device(list_next->this.path, O_RDWR);

        if(!camdev)
        {
            dirent = readdir(dir);
            free(list_next);
            continue;
        }

        camccb = cam_getccb(camdev);

        if(!camccb)
        {
            cam_close_device(camdev);
            dirent = readdir(dir);
            free(list_next);
            continue;
        }

        camccb->ccb_h.func_code = XPT_GDEV_TYPE;

        ret = cam_send_ccb(camdev, camccb);

        if(ret < 0)
        {
            dirent = readdir(dir);
            cam_freeccb(camccb);
            cam_close_device(camdev);
            free(list_next);
            continue;
        }

        strncpy(list_next->this.serial, (const char *)camdev->serial_num, camdev->serial_num_len);

#ifndef HAS_CAM_ATA
        // Before FreeBSD 8 CAM has no protocol information, and only has SCSI devices and ATAPI ones through atapicam
        FillScsiDeviceInfo(&list_next->this, camccb, camdev);
#else
        switch(camccb->cgd.protocol)
        {
            case PROTO_ATA:
            case PROTO_ATAPI:
            case PROTO_SATAPM:
                // TODO: Split on space
                strncpy(list_next->this.vendor, "ATA", 3);
                strncpy(list_next->this.model, (const char *)camccb->cgd.ident_data.model, 40);

                // Trim spaces
                for(i = 39; i >= 0; i--)
                {
                    if(list_next->this.model[i] != 0x20 && list_next->this.model[i] != 0) break;

                    list_next->this.model[i] = 0;
                }

                strncpy(list_next->this.serial, (const char *)camccb->cgd.ident_data.serial, 20);

                if(strncmp(camdev->sim_name, "ahcich", 6) == 0)
                    strncpy(list_next->this.bus, "SATA", 5);
                else
                    strncpy(list_next->this.bus, "ATA", 4);

                list_next->this.supported = true;

                if(camccb->cgd.protocol == PROTO_ATAPI) FillScsiDeviceInfo(&list_next->this, camccb, camdev);

                break;
            case PROTO_SCSI:
                FillScsiDeviceInfo(&list_next->this, camccb, camdev);
                break;
#if __FreeBSD_version >= 1200000
            case PROTO_NVME:
                strncpy(list_next->this.bus, "NVMe", 4);
                list_next->this.supported = false;
                break;
            case PROTO_MMCSD:
                strncpy(list_next->this.bus, "MMC/SD", 6);
                list_next->this.supported = false;
                break;
#endif
            default:
                dirent = readdir(dir);
                cam_freeccb(camccb);
                cam_close_device(camdev);
                free(list_next);
                continue;
        }
#endif

        cam_freeccb(camccb);
        cam_close_device(camdev);

        // Only link devices that were probed successfully, the others have been freed
        if(!list_start) list_start = list_next;

        if(list_current) list_current->next = list_next;

        list_current = list_next;
        dirent       = readdir(dir);
    }

    closedir(dir);

#if defined(HAS_ATA_IOCTL) || defined(HAS_ATA_DEVICE_IOCTL)
    list_current = ListAtaDevices(&list_start, list_current);
#endif

    return list_start;
}
