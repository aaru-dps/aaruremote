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

int32_t linux_get_sdhci_registers(const char* devicePath,
                                  char**      csd,
                                  char**      cid,
                                  char**      ocr,
                                  char**      scr,
                                  uint32_t*   csd_len,
                                  uint32_t*   cid_len,
                                  uint32_t*   ocr_len,
                                  uint32_t*   scr_len)
{
    char*  tmpString;
    char*  sysfsPath_csd;
    char*  sysfsPath_cid;
    char*  sysfsPath_scr;
    char*  sysfsPath_ocr;
    size_t len;
    FILE*  file;
    *csd     = NULL;
    *cid     = NULL;
    *ocr     = NULL;
    *scr     = NULL;
    *csd_len = 0;
    *cid_len = 0;
    *ocr_len = 0;
    *scr_len = 0;
    size_t n = 1026;

    if(strncmp(devicePath, "/dev/mmcblk", 11) != 0) return 0;

    len           = strlen(devicePath) + 19;
    sysfsPath_csd = malloc(len);
    sysfsPath_cid = malloc(len);
    sysfsPath_scr = malloc(len);
    sysfsPath_ocr = malloc(len);
    tmpString     = malloc(1024);

    if(!sysfsPath_csd || !sysfsPath_cid || !sysfsPath_scr || !sysfsPath_ocr || !tmpString)
    {
        free(sysfsPath_csd);
        free(sysfsPath_cid);
        free(sysfsPath_scr);
        free(sysfsPath_ocr);
        free(tmpString);
        return 0;
    }

    memset(sysfsPath_csd, 0, len);
    memset(sysfsPath_cid, 0, len);
    memset(sysfsPath_scr, 0, len);
    memset(sysfsPath_ocr, 0, len);
    memset(tmpString, 0, strlen(devicePath) - 5);

    memcpy(tmpString, devicePath + 5, strlen(devicePath) - 5);
    snprintf(sysfsPath_csd, len, "/sys/block/%s/device/csd", tmpString);
    snprintf(sysfsPath_cid, len, "/sys/block/%s/device/cid", tmpString);
    snprintf(sysfsPath_scr, len, "/sys/block/%s/device/scr", tmpString);
    snprintf(sysfsPath_ocr, len, "/sys/block/%s/device/ocr", tmpString);

    if(access(sysfsPath_csd, R_OK) == 0)
    {
        file = fopen(sysfsPath_csd, "r");

        if(file != NULL)
        {
            len = getline(&tmpString, &n, file);
            if(len > 0)
            {
                *csd_len = hexs2bin(tmpString, (unsigned char**)csd);

                if(*csd_len <= 0)
                {
                    *csd_len = 0;
                    *csd     = NULL;
                }
            }

            fclose(file);
        }
    }

    if(access(sysfsPath_cid, R_OK) == 0)
    {
        file = fopen(sysfsPath_cid, "r");

        if(file != NULL)
        {
            len = getline(&tmpString, &n, file);
            if(len > 0)
            {
                *cid_len = hexs2bin(tmpString, (unsigned char**)cid);

                if(*cid_len <= 0)
                {
                    *cid_len = 0;
                    *cid     = NULL;
                }
            }

            fclose(file);
        }
    }

    if(access(sysfsPath_scr, R_OK) == 0)
    {
        file = fopen(sysfsPath_scr, "r");

        if(file != NULL)
        {
            len = getline(&tmpString, &n, file);
            if(len > 0)
            {
                *scr_len = hexs2bin(tmpString, (unsigned char**)scr);

                if(*scr_len <= 0)
                {
                    *scr_len = 0;
                    *scr     = NULL;
                }
            }

            fclose(file);
        }
    }

    if(access(sysfsPath_ocr, R_OK) == 0)
    {
        file = fopen(sysfsPath_ocr, "r");

        if(file != NULL)
        {
            len = getline(&tmpString, &n, file);
            if(len > 0)
            {
                *ocr_len = hexs2bin(tmpString, (unsigned char**)ocr);

                if(*ocr_len <= 0)
                {
                    *ocr_len = 0;
                    *ocr     = NULL;
                }
            }

            fclose(file);
        }
    }

    free(sysfsPath_csd);
    free(sysfsPath_cid);
    free(sysfsPath_scr);
    free(sysfsPath_ocr);
    free(tmpString);

    return csd_len != 0 || cid_len != 0 || scr_len != 0 || ocr_len != 0;
}