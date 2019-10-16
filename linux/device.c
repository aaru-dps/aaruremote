/*
 * This file is part of the DiscImageChef Remote Server.
 * Copyright (c) 2019 Natalia Portillo.
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

#include "linux.h"

#include <errno.h>
#include <fcntl.h>
#include <libudev.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int linux_open_device(const char* device_path)
{
    int fd;

    fd = open(device_path, O_RDWR | O_NONBLOCK | O_CREAT);

    if((fd < 0) && (errno == EACCES || errno == EROFS)) { fd = open(device_path, O_RDONLY | O_NONBLOCK); }

    return fd;
}

int32_t linux_get_device_type(const char* devicePath)
{
    struct udev*        udev;
    struct udev_device* udev_device;
    const char*         tmpString;
    char*               chrptr;
    int32_t             deviceType = DICMOTE_DEVICE_TYPE_UNKNOWN;

    udev = udev_new();

    if(!udev) return DICMOTE_DEVICE_TYPE_UNKNOWN;

    chrptr = strrchr(devicePath, '/');
    if(chrptr == 0) return DICMOTE_DEVICE_TYPE_UNKNOWN;

    chrptr++;
    if(chrptr == 0) return DICMOTE_DEVICE_TYPE_UNKNOWN;

    udev_device = udev_device_new_from_subsystem_sysname(udev, "block", chrptr);
    if(udev_device)
    {
        tmpString = udev_device_get_property_value(udev_device, "ID_BUS");
        if(tmpString)
        {
            if(strncmp(tmpString, "ata", 3) == 0)
            {
                deviceType = DICMOTE_DEVICE_TYPE_ATA;

                free((void*)tmpString);
                tmpString = udev_device_get_property_value(udev_device, "ID_TYPE");

                if(tmpString)
                {
                    // TODO: ATAPI removable non optical disks
                    if(strncmp(tmpString, "cd", 2) == 0) { deviceType = DICMOTE_DEVICE_TYPE_ATAPI; }

                    free((void*)tmpString);
                }
            }
            else if(strncmp(tmpString, "mmc", 3) == 0)
            {
                free((void*)tmpString);
                tmpString = malloc(1024);

                deviceType = DICMOTE_DEVICE_TYPE_MMC;

                if(tmpString)
                {
                    memset((void*)tmpString, 0, 1024);
                    snprintf((char*)tmpString, 1024, "/sys/block/%s/device/scr", chrptr);

                    if(access(tmpString, R_OK) == 0) deviceType = DICMOTE_DEVICE_TYPE_SECURE_DIGITAL;

                    free((void*)tmpString);
                }
            }
            else if(strncmp(tmpString, "scsi", 4) == 0 || strncmp(tmpString, "ieee1394", 8) == 0 ||
                    strncmp(tmpString, "usb", 3) == 0)
            {
                free((void*)tmpString);
                tmpString = udev_device_get_property_value(udev_device, "ID_TYPE");

                if(tmpString)
                {
                    if(strncmp(tmpString, "cd", 2) == 0 || strncmp(tmpString, "disk", 4) == 0)
                    { deviceType = DICMOTE_DEVICE_TYPE_SCSI; }

                    free((void*)tmpString);
                }
            }
            else if(strncmp(tmpString, "nvme", 4) == 0)
            {
                free((void*)tmpString);
                deviceType = DICMOTE_DEVICE_TYPE_NVME;
            }
        }
    }

    udev_unref(udev);

    return deviceType;
}