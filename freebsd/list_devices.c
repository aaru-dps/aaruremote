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

#include <dirent.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../aaruremote.h"
#include "freebsd.h"

DeviceInfoList* ListDevices()
{
    DeviceInfoList*    list_start = NULL, *list_current = NULL, *list_next = NULL;
    DIR*               dir;
    struct dirent*     dirent;
    struct cam_device* camdev;
    union ccb* camccb;
    int ret;
    int i;

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
        memset(list_next, 0, sizeof(DeviceInfoList));

        if(!list_start) list_start = list_next;

        if(list_current) list_current->next = list_next;

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

        strncpy(list_next->this.serial, (const char*)camdev->serial_num, camdev->serial_num_len);

        switch(camccb->cgd.protocol)
        {
            case PROTO_ATA:
            case PROTO_ATAPI:
            case PROTO_SATAPM:
                // TODO: Split on space
                strncpy(list_next->this.vendor, "ATA", 3);
                strncpy(list_next->this.model, (const char *)camccb->cgd.ident_data.model, 40);

                // Trim spaces
                for(i = 40; i > 0; i--)
                {
                    if(list_next->this.model[i] != 0x20) break;

                    list_next->this.model[i] = 0;
                }

                strncpy(list_next->this.serial, (const char *)camccb->cgd.ident_data.serial, 20);

                if(strncmp(camdev->sim_name, "ahcich", 6) == 0)
                    strncpy(list_next->this.bus, "SATA", 5);
                else
                    strncpy(list_next->this.bus, "ATA", 4);

                // TODO: This protocol didn't work in C#, does it work in C?
                list_next->this.supported = strncmp(camdev->sim_name, "ata", 3) != 0;

                if(camccb->cgd.protocol == PROTO_ATAPI)
                    goto protocol_atapi;

                break;
            case PROTO_SCSI:
            protocol_atapi:
                strncpy(list_next->this.vendor, camccb->cgd.inq_data.vendor, 8);
                strncpy(list_next->this.model, camccb->cgd.inq_data.product, 16);

                if(strncmp(camdev->sim_name, "ata", 3) == 0 ||
                    strncmp(camdev->sim_name, "ahcich", 6) == 0)
                    strncpy(list_next->this.bus, "ATAPI", 5);
                else
                    strncpy(list_next->this.bus, "SCSI", 4);

                // TODO: This protocol didn't work in C#, does it work in C?
                list_next->this.supported = strncmp(camdev->sim_name, "ata", 3) != 0;

                break;
            case PROTO_NVME:
                strncpy(list_next->this.bus, "NVMe", 4);
                list_next->this.supported = false;
                break;
            case PROTO_MMCSD:
                strncpy(list_next->this.bus, "MMC/SD", 6);
                list_next->this.supported = false;
                break;
            default:
                dirent = readdir(dir);
                free(list_next);
                continue;
        }

        cam_freeccb(camccb);
        cam_close_device(camdev);

        list_current = list_next;
        dirent       = readdir(dir);
    }

    closedir(dir);

    return list_start;
}