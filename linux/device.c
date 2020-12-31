/*
 * This file is part of the Aaru Remote Server.
 * Copyright (c) 2019-2021 Natalia Portillo.
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
#include <unistd.h>

#ifdef HAS_UDEV
#include <libudev.h>
#endif

#include "../aaruremote.h"
#include "linux.h"

void* DeviceOpen(const char* device_path)
{
    DeviceContext* ctx;

    ctx = malloc(sizeof(DeviceContext));

    if(!ctx) return NULL;

    memset(ctx, 0, sizeof(DeviceContext));

    ctx->fd = open(device_path, O_RDWR | O_NONBLOCK | O_CREAT);

    if((ctx->fd < 0) && (errno == EACCES || errno == EROFS)) { ctx->fd = open(device_path, O_RDONLY | O_NONBLOCK); }

    if(ctx->fd <= 0)
    {
        free(ctx);
        return NULL;
    }

    strncpy(ctx->device_path, device_path, 4096);

    return ctx;
}

void DeviceClose(void* device_ctx)
{
    DeviceContext* ctx = device_ctx;

    if(!ctx) return;

    close(ctx->fd);

    free(ctx);
}

int32_t GetDeviceType(void* device_ctx)
{
    DeviceContext* ctx = device_ctx;

    if(!ctx) return -1;

#ifdef HAS_UDEV
    struct udev*        udev;
    struct udev_device* udev_device;
    const char*         tmp_string;
    char*               chrptr;
    int32_t             device_type = AARUREMOTE_DEVICE_TYPE_UNKNOWN;

    udev = udev_new();

    if(!udev) return AARUREMOTE_DEVICE_TYPE_UNKNOWN;

    chrptr = strrchr(ctx->device_path, '/');
    if(chrptr == 0) return AARUREMOTE_DEVICE_TYPE_UNKNOWN;

    chrptr++;
    if(chrptr == 0) return AARUREMOTE_DEVICE_TYPE_UNKNOWN;

    udev_device = udev_device_new_from_subsystem_sysname(udev, "block", chrptr);
    if(udev_device)
    {
        tmp_string = udev_device_get_property_value(udev_device, "ID_BUS");
        if(tmp_string)
        {
            if(strncmp(tmp_string, "ata", 3) == 0)
            {
                device_type = AARUREMOTE_DEVICE_TYPE_ATA;

                free((void*)tmp_string);
                tmp_string = udev_device_get_property_value(udev_device, "ID_TYPE");

                if(tmp_string)
                {
                    // TODO: ATAPI removable non optical disks
                    if(strncmp(tmp_string, "cd", 2) == 0) { device_type = AARUREMOTE_DEVICE_TYPE_ATAPI; }

                    free((void*)tmp_string);
                }
            }
            else if(strncmp(tmp_string, "mmc", 3) == 0)
            {
                free((void*)tmp_string);
                tmp_string = malloc(1024);

                device_type = AARUREMOTE_DEVICE_TYPE_MMC;

                if(tmp_string)
                {
                    memset((void*)tmp_string, 0, 1024);
                    snprintf((char*)tmp_string, 1024, "/sys/block/%s/device/scr", chrptr);

                    if(access(tmp_string, R_OK) == 0) device_type = AARUREMOTE_DEVICE_TYPE_SECURE_DIGITAL;

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
                    if(strncmp(tmp_string, "cd", 2) == 0 || strncmp(tmp_string, "disk", 4) == 0 ||
                       strncmp(tmp_string, "optical", 7) == 0)
                        device_type = AARUREMOTE_DEVICE_TYPE_SCSI;

                    free((void*)tmp_string);
                }
            }
            else if(strncmp(tmp_string, "nvme", 4) == 0)
            {
                free((void*)tmp_string);
                device_type = AARUREMOTE_DEVICE_TYPE_NVME;
            }
        }
    }

    udev_unref(udev);

    return device_type;
#else
    int32_t     dev_type = AARUREMOTE_DEVICE_TYPE_UNKNOWN;
    const char* dev_name;
    const char* sysfs_path;
    char*       dev_path;
    char*       host_no;
    char*       scsi_path;
    char*       iscsi_path;
    char*       spi_path;
    char*       fc_path;
    char*       sas_path;
    int         ret;
    char*       chrptr;
    char*       sysfs_path_scr;
    FILE*       file;
    size_t      len = 4096;

    if(strlen(ctx->device_path) <= 5) return dev_type;

    if(strstr(ctx->device_path, "nvme")) return AARUREMOTE_DEVICE_TYPE_NVME;

    dev_name = ctx->device_path + 5;

    if(strstr(ctx->device_path, "mmcblk"))
    {
        dev_type = AARUREMOTE_DEVICE_TYPE_MMC;

        sysfs_path_scr = malloc(len);

        if(sysfs_path_scr)
        {
            snprintf(sysfs_path_scr, len, "/sys/block/%s/device/scr", dev_name);

            if(access(sysfs_path_scr, F_OK) == 0) dev_type = AARUREMOTE_DEVICE_TYPE_SECURE_DIGITAL;

            free(sysfs_path_scr);
        }

        return dev_type;
    }

    sysfs_path = malloc(len);
    dev_path   = malloc(len);
    host_no    = malloc(len);
    scsi_path  = malloc(len);
    iscsi_path = malloc(len);
    spi_path   = malloc(len);
    fc_path    = malloc(len);
    sas_path   = malloc(len);

    if(!sysfs_path || !dev_path || !host_no || !scsi_path || !iscsi_path || !spi_path || !fc_path || !sas_path)
    {
        free((void*)sysfs_path);
        free((void*)dev_path);
        free((void*)host_no);
        free((void*)iscsi_path);
        free((void*)scsi_path);
        free((void*)spi_path);
        free((void*)fc_path);
        free((void*)sas_path);
        return dev_type;
    }

    memset((void*)sysfs_path, 0, len);
    memset((void*)dev_path, 0, len);
    memset((void*)host_no, 0, len);
    memset((void*)iscsi_path, 0, len);
    memset((void*)scsi_path, 0, len);
    memset((void*)spi_path, 0, len);
    memset((void*)fc_path, 0, len);
    memset((void*)sas_path, 0, len);

    snprintf((char*)sysfs_path, len, "%s/%s/device", PATH_SYS_DEVBLOCK, dev_name);

    ret = readlink(sysfs_path, dev_path, len);

    if(ret <= 0)
    {
        free((void*)sysfs_path);
        free((void*)dev_path);
        free((void*)host_no);
        free((void*)iscsi_path);
        free((void*)scsi_path);
        free((void*)spi_path);
        free((void*)fc_path);
        free((void*)sas_path);
        return dev_type;
    }

    ret    = 0;
    chrptr = strchr(dev_path, ':') - 1;

    while(chrptr != dev_path)
    {
        if(chrptr[0] == '/')
        {
            chrptr++;
            break;
        }

        ret++;
        chrptr--;
    }

    memcpy((void*)host_no, chrptr, ret);

    snprintf(spi_path, len, "/sys/class/spi_host/host%s", host_no);
    snprintf(fc_path, len, "/sys/class/fc_host/host%s", host_no);
    snprintf(sas_path, len, "/sys/class/sas_host/host%s", host_no);
    snprintf(iscsi_path, len, "/sys/class/iscsi_host/host%s", host_no);
    snprintf(scsi_path, len, "/sys/class/scsi_host/host%s", host_no);

    if(access(spi_path, F_OK) == 0 || access(fc_path, F_OK) == 0 || access(sas_path, F_OK) == 0 ||
       access(iscsi_path, F_OK) == 0)
        dev_type = AARUREMOTE_DEVICE_TYPE_SCSI;
    else if(access(scsi_path, F_OK) == 0)
    {
        dev_type = AARUREMOTE_DEVICE_TYPE_SCSI;
        memset(scsi_path, 0, len);
        snprintf(scsi_path, len, "/sys/class/scsi_host/host%s/proc_name", host_no);
        if(access(scsi_path, F_OK) == 0)
        {
            file = fopen(scsi_path, "r");
            if(file)
            {
                memset(scsi_path, 0, len);
                ret = getline(&scsi_path, &len, file);

                len = 4096;

                fclose(file);

                if(ret > 0)
                {
                    if(strncmp(scsi_path, "ata", 3) == 0 || strncmp(scsi_path, "sata", 4) == 0 ||
                       strncmp(scsi_path, "ahci", 4) == 0)
                    {
                        dev_type = AARUREMOTE_DEVICE_TYPE_ATA;
                        memset(scsi_path, 0, len);
                        snprintf(scsi_path, len, "%s/%s/removable", PATH_SYS_DEVBLOCK, dev_name);

                        file = fopen(scsi_path, "r");
                        if(file)
                        {
                            ret = (size_t)fread(scsi_path, 1, 1, file);
                            if(ret == 1)
                            {
                                if(scsi_path[0] == '1') dev_type = AARUREMOTE_DEVICE_TYPE_ATAPI;
                            }
                            fclose(file);
                        }
                    }
                }
            }
        }
    }

    free((void*)sysfs_path);
    free((void*)dev_path);
    free((void*)host_no);
    free((void*)iscsi_path);
    free((void*)scsi_path);
    free((void*)spi_path);
    free((void*)fc_path);
    free((void*)sas_path);

    return dev_type;
#endif
}

int32_t ReOpen(void* device_ctx, uint32_t* closeFailed)
{
    DeviceContext* ctx = device_ctx;
    int            ret;
    *closeFailed = 0;

    if(!ctx) return -1;

    ret = close(ctx->fd);

    if(ret < 0)
    {
        *closeFailed = 1;
        return errno;
    }

    ctx->fd = open(ctx->device_path, O_RDWR | O_NONBLOCK | O_CREAT);

    if((ctx->fd < 0) && (errno == EACCES || errno == EROFS)) ctx->fd = open(ctx->device_path, O_RDONLY | O_NONBLOCK);

    return ctx->fd <= 0 ? errno : 0;
}

int32_t OsRead(void* device_ctx, char* buffer, uint64_t offset, uint32_t length, uint32_t* duration)
{
    DeviceContext* ctx = device_ctx;
    ssize_t        ret;
    *duration = 0;
    off_t pos;

    if(!ctx) return -1;

    // TODO: Timing
    pos = lseek(ctx->fd, (off_t)offset, SEEK_SET);

    if(pos < 0) return errno;

    // TODO: Timing
    ret = read(ctx->fd, (void*)buffer, (size_t)length);

    return ret < 0 ? errno : 0;
}