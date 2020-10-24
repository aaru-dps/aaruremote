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

#include <gccore.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../aaruremote.h"
#include "wii.h"

DeviceInfoList* ListDevices()
{
    DeviceInfoList *list_start = NULL, *list_current = NULL, *list_next = NULL;
    u32             deviceId = 0;
    s32             sd_fd;

    list_next = malloc(sizeof(DeviceInfoList));
    memset(list_next, 0, sizeof(DeviceInfoList));

    // All Wiis do have flash
    strncpy(list_next->this.path, AARUREMOTE_WII_DEVICE_PATH_NAND, 1024);
    strncpy(list_next->this.vendor, "Nintendo", 256);
    strncpy(list_next->this.model, "Wii NAND", 256);
    strncpy(list_next->this.bus, "NAND", 256);
    list_next->this.supported = false; // TODO: Implement NAND reading

    if(ES_GetDeviceID(&deviceId) < 0) snprintf(list_next->this.serial, 256, "%d", deviceId);

    list_start   = list_next;
    list_current = list_start;

    sd_fd = IOS_Open(AARUREMOTE_WII_DEVICE_PATH_SD, IPC_OPEN_READ);

    if(sd_fd >= 0)
    {
        deviceId = 0;
        IOS_Ioctl(sd_fd, AARUREMOTE_WII_IOCTL_SD_GET_DEVICE_STATUS, 0, 0, &deviceId, sizeof(deviceId));

        if(deviceId == AARUREMOTE_WII_SD_INSERTED)
        {
            strncpy(list_next->this.path, "/dev/flash", 1024);
            strncpy(list_next->this.vendor, "Nintendo", 256);
            strncpy(list_next->this.model, "Wii SD", 256);
            strncpy(list_next->this.bus, "MMC/SD", 256);
            list_next->this.supported = false; // TODO: Implement SD/MMC reading
            list_current->next        = list_next;
            list_current              = list_next;
        }

        IOS_Close(sd_fd);
    }

    return list_start;
}