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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef HAS_UDEV
#include <libudev.h>
#endif

int LinuxOpenDevice(const char* device_path)
{
    int fd;

    fd = open(device_path, O_RDWR | O_NONBLOCK | O_CREAT);

    if((fd < 0) && (errno == EACCES || errno == EROFS)) { fd = open(device_path, O_RDONLY | O_NONBLOCK); }

    return fd;
}

int32_t LinuxGetDeviceType(const char* device_path)
{
#ifndef HAS_UDEV
    return DICMOTE_DEVICE_TYPE_UNKNOWN;
#else
    struct udev*        udev;
    struct udev_device* udev_device;
    const char*         tmp_string;
    char*               chrptr;
    int32_t             device_type = DICMOTE_DEVICE_TYPE_UNKNOWN;

    udev = udev_new();

    if(!udev) return DICMOTE_DEVICE_TYPE_UNKNOWN;

    chrptr = strrchr(device_path, '/');
    if(chrptr == 0) return DICMOTE_DEVICE_TYPE_UNKNOWN;

    chrptr++;
    if(chrptr == 0) return DICMOTE_DEVICE_TYPE_UNKNOWN;

    udev_device = udev_device_new_from_subsystem_sysname(udev, "block", chrptr);
    if(udev_device)
    {
        tmp_string = udev_device_get_property_value(udev_device, "ID_BUS");
        if(tmp_string)
        {
            if(strncmp(tmp_string, "ata", 3) == 0)
            {
                device_type = DICMOTE_DEVICE_TYPE_ATA;

                free((void*)tmp_string);
                tmp_string = udev_device_get_property_value(udev_device, "ID_TYPE");

                if(tmp_string)
                {
                    // TODO: ATAPI removable non optical disks
                    if(strncmp(tmp_string, "cd", 2) == 0) { device_type = DICMOTE_DEVICE_TYPE_ATAPI; }

                    free((void*)tmp_string);
                }
            }
            else if(strncmp(tmp_string, "mmc", 3) == 0)
            {
                free((void*)tmp_string);
                tmp_string = malloc(1024);

                device_type = DICMOTE_DEVICE_TYPE_MMC;

                if(tmp_string)
                {
                    memset((void*)tmp_string, 0, 1024);
                    snprintf((char*)tmp_string, 1024, "/sys/block/%s/device/scr", chrptr);

                    if(access(tmp_string, R_OK) == 0) device_type = DICMOTE_DEVICE_TYPE_SECURE_DIGITAL;

                    free((void*)tmp_string);
                }
            }
            else if(strncmp(tmp_string, "scsi", 4) == 0 || strncmp(tmp_string, "ieee1394", 8) == 0 ||
                    strncmp(tmp_string, "usb", 3) == 0)
            {
                free((void*)tmp_string);
                tmp_string = udev_device_get_property_value(udev_device, "ID_TYPE");

                if(tmp_string)
                {
                    if(strncmp(tmp_string, "cd", 2) == 0 || strncmp(tmp_string, "disk", 4) == 0)
                    { device_type = DICMOTE_DEVICE_TYPE_SCSI; }

                    free((void*)tmp_string);
                }
            }
            else if(strncmp(tmp_string, "nvme", 4) == 0)
            {
                free((void*)tmp_string);
                device_type = DICMOTE_DEVICE_TYPE_NVME;
            }
        }
    }

    udev_unref(udev);

    return device_type;
}

int32_t LinuxGetSdhciRegisters(const char* device_path,
                               char** csd,
                               char** cid,
                               char** ocr,
                               char** scr,
                               uint32_t* csd_len,
                               uint32_t* cid_len,
                               uint32_t* ocr_len,
                               uint32_t* scr_len)
{
    char* tmp_string;
    char* sysfs_path_csd;
    char* sysfs_path_cid;
    char* sysfs_path_scr;
    char* sysfs_path_ocr;
    size_t len;
    FILE* file;
    *csd = NULL;
    *cid = NULL;
    *ocr = NULL;
    *scr = NULL;
    *csd_len = 0;
    *cid_len = 0;
    *ocr_len = 0;
    *scr_len = 0;
    size_t n = 1026;

    if(strncmp(device_path, "/dev/mmcblk", 11) != 0) return 0;

    len = strlen(device_path) + 19;
    sysfs_path_csd = malloc(len);
    sysfs_path_cid = malloc(len);
    sysfs_path_scr = malloc(len);
    sysfs_path_ocr = malloc(len);
    tmp_string = malloc(1024);

    if(!sysfs_path_csd || !sysfs_path_cid || !sysfs_path_scr || !sysfs_path_ocr || !tmp_string)
    {
        free(sysfs_path_csd);
        free(sysfs_path_cid);
        free(sysfs_path_scr);
        free(sysfs_path_ocr);
        free(tmp_string);
        return 0;
    }

    memset(sysfs_path_csd, 0, len);
    memset(sysfs_path_cid, 0, len);
    memset(sysfs_path_scr, 0, len);
    memset(sysfs_path_ocr, 0, len);
    memset(tmp_string, 0, strlen(device_path) - 5);

    memcpy(tmp_string, device_path + 5, strlen(device_path) - 5);
    snprintf(sysfs_path_csd, len, "/sys/block/%s/device/csd", tmp_string);
    snprintf(sysfs_path_cid, len, "/sys/block/%s/device/cid", tmp_string);
    snprintf(sysfs_path_scr, len, "/sys/block/%s/device/scr", tmp_string);
    snprintf(sysfs_path_ocr, len, "/sys/block/%s/device/ocr", tmp_string);

    if(access(sysfs_path_csd, R_OK) == 0)
    {
        file = fopen(sysfs_path_csd, "r");

        if(file != NULL)
        {
            len = getline(&tmp_string, &n, file);
            if(len > 0)
            {
                *csd_len = Hexs2Bin(tmp_string, (unsigned char**)csd);

                if(*csd_len <= 0)
                {
                    *csd_len = 0;
                    *csd = NULL;
                }
            }

            fclose(file);
        }
    }

    if(access(sysfs_path_cid, R_OK) == 0)
    {
        file = fopen(sysfs_path_cid, "r");

        if(file != NULL)
        {
            len = getline(&tmp_string, &n, file);
            if(len > 0)
            {
                *cid_len = Hexs2Bin(tmp_string, (unsigned char**)cid);

                if(*cid_len <= 0)
                {
                    *cid_len = 0;
                    *cid = NULL;
                }
            }

            fclose(file);
        }
    }

    if(access(sysfs_path_scr, R_OK) == 0)
    {
        file = fopen(sysfs_path_scr, "r");

        if(file != NULL)
        {
            len = getline(&tmp_string, &n, file);
            if(len > 0)
            {
                *scr_len = Hexs2Bin(tmp_string, (unsigned char**)scr);

                if(*scr_len <= 0)
                {
                    *scr_len = 0;
                    *scr = NULL;
                }
            }

            fclose(file);
        }
    }

    if(access(sysfs_path_ocr, R_OK) == 0)
    {
        file = fopen(sysfs_path_ocr, "r");

        if(file != NULL)
        {
            len = getline(&tmp_string, &n, file);
            if(len > 0)
            {
                *ocr_len = Hexs2Bin(tmp_string, (unsigned char**)ocr);

                if(*ocr_len <= 0)
                {
                    *ocr_len = 0;
                    *ocr = NULL;
                }
            }

            fclose(file);
        }
    }

    free(sysfs_path_csd);
    free(sysfs_path_cid);
    free(sysfs_path_scr);
    free(sysfs_path_ocr);
    free(tmp_string);

    return csd_len != 0 || cid_len != 0 || scr_len != 0 || ocr_len != 0;
#endif
}