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

#ifdef HAS_UDEV
#include <libudev.h>
#endif

#include <ctype.h>
#include <dirent.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../aaruremote.h"
#include "linux.h"

DeviceInfoList* ListDevices()
{
    DIR*            dir;
    struct dirent*  dirent;
    int             i;
    DeviceInfoList *list_start = NULL, *list_current = NULL, *list_next = NULL;
    const char*     tmp_string;
    FILE*           file;
    char*           line_str;
    size_t          n, ret;
    char*           chrptr;
    int             has_udev = 0;
#ifdef HAS_UDEV
    struct udev*        udev;
    struct udev_device* udev_device;

    udev     = udev_new();
    has_udev = udev != 0;
#else
    DeviceContext   tmp_ctx;
#endif

    dir = opendir(PATH_SYS_DEVBLOCK);
    if(!dir) return NULL;

    dirent = readdir(dir);

    while(dirent)
    {
        if((dirent->d_type != DT_DIR && dirent->d_type != DT_LNK) || dirent->d_name[0] == '.')
        {
            dirent = readdir(dir);
            continue;
        }

        list_next = malloc(sizeof(DeviceInfoList));
        memset(list_next, 0, sizeof(DeviceInfoList));

        if(!list_next)
        {
            closedir(dir);

#ifdef HAS_UDEV
            if(has_udev) udev_unref(udev);
#endif
            return list_start;
        }

        if(!list_start) list_start = list_next;

        if(list_current) list_current->next = list_next;

        snprintf(list_next->this.path, 1024, "/dev/%s", dirent->d_name);

#ifdef HAS_UDEV
        if(has_udev)
        {
            udev_device = udev_device_new_from_subsystem_sysname(udev, "block", dirent->d_name);
            if(udev_device)
            {
                tmp_string = udev_device_get_property_value(udev_device, "ID_VENDOR");
                if(tmp_string)
                {
                    strncpy(list_next->this.vendor, tmp_string, 256);
                    free((void*)tmp_string);
                }

                tmp_string = udev_device_get_property_value(udev_device, "ID_MODEL");
                if(tmp_string)
                {
                    strncpy(list_next->this.model, tmp_string, 256);
                    free((void*)tmp_string);

                    for(i = 0; i < 256; i++)
                    {
                        if(list_next->this.model[i] == 0) break;

                        if(list_next->this.model[i] == '_') list_next->this.model[i] = ' ';
                    }
                }

                tmp_string = udev_device_get_property_value(udev_device, "ID_SCSI_SERIAL");
                if(tmp_string)
                {
                    strncpy(list_next->this.serial, tmp_string, 256);
                    free((void*)tmp_string);
                }
                else
                {
                    tmp_string = udev_device_get_property_value(udev_device, "ID_SERIAL_SHORT");
                    if(tmp_string)
                    {
                        strncpy(list_next->this.serial, tmp_string, 256);
                        free((void*)tmp_string);
                    }
                }

                tmp_string = udev_device_get_property_value(udev_device, "ID_BUS");
                if(tmp_string)
                {
                    strncpy(list_next->this.bus, tmp_string, 256);
                    free((void*)tmp_string);
                }
            }
        }
#else // Use sysfs
        if(!has_udev && !strstr(dirent->d_name, "loop"))
        {
            memset((void*)tmp_ctx.device_path, 0, 4096);
            snprintf((char*)tmp_ctx.device_path, 4096, "/dev/%s", dirent->d_name);

            switch(GetDeviceType(&tmp_ctx))
            {
                case AARUREMOTE_DEVICE_TYPE_ATA: strncpy(list_next->this.bus, "ATA", 256); break;
                case AARUREMOTE_DEVICE_TYPE_ATAPI: strncpy(list_next->this.bus, "ATAPI", 256); break;
                case AARUREMOTE_DEVICE_TYPE_MMC:
                case AARUREMOTE_DEVICE_TYPE_SECURE_DIGITAL: strncpy(list_next->this.bus, "MMC/SD", 256); break;
                case AARUREMOTE_DEVICE_TYPE_NVME: strncpy(list_next->this.bus, "NVMe", 256); break;
                case AARUREMOTE_DEVICE_TYPE_SCSI:
                    tmp_string = malloc(1024);
                    memset((void*)tmp_string, 0, 1024);
                    snprintf((char*)tmp_string, 1024, "%s/%s/device", PATH_SYS_DEVBLOCK, dirent->d_name);
                    line_str = malloc(1024);
                    memset(line_str, 0, 1024);

                    ret = readlink(tmp_string, line_str, 1024);

                    if(ret > 0)
                    {
                        ret    = 0;
                        chrptr = strchr(line_str, ':') - 1;

                        while(chrptr != line_str)
                        {
                            if(chrptr[0] == '/')
                            {
                                chrptr++;
                                break;
                            }

                            ret++;
                            chrptr--;
                        }

                        memset((void*)tmp_string, 0, 1024);
                        memcpy((void*)tmp_string, chrptr, ret);
                        snprintf((char*)line_str, 1024, "/sys/class/scsi_host/host%s/proc_name", tmp_string);
                        memset((void*)tmp_string, 0, 1024);

                        file = fopen(line_str, "r");
                        if(file)
                        {
                            n   = 1024;
                            ret = getline(&line_str, &n, file);

                            if(ret > 0)
                            {
                                if(strncmp(line_str, "sbp2", 4) == 0) strncpy(list_next->this.bus, "FireWire", 256);
                                else if(strncmp(line_str, "usb-storage", 11) == 0)
                                    strncpy(list_next->this.bus, "USB", 256);
                                else
                                    strncpy(list_next->this.bus, "SCSI", 256);
                            }
                            else
                                strncpy(list_next->this.bus, "SCSI", 256);

                            fclose(file);
                        }
                        else
                            strncpy(list_next->this.bus, "SCSI", 256);

                        free(line_str);
                    }
                    else
                    {
                        strncpy(list_next->this.bus, "SCSI", 256);
                        free(line_str);
                    }

                    free((void*)tmp_string);
                    break;
                default: memset(&list_next->this.bus, 0, 256); break;
            }
        }
#endif

        tmp_string = malloc(1024);
        memset((void*)tmp_string, 0, 1024);
        snprintf((char*)tmp_string, 1024, "%s/%s/device/vendor", PATH_SYS_DEVBLOCK, dirent->d_name);

        if(access(tmp_string, R_OK) == 0 && strlen(list_next->this.vendor) == 0)
        {
            file = fopen(tmp_string, "rb");

            if(file != NULL)
            {
                line_str = malloc(256);
                memset(line_str, 0, 256);
                n   = 256;
                ret = getline(&line_str, &n, file);

                if(ret > 0 && line_str != NULL)
                {
                    strncpy(list_next->this.vendor, line_str, 256);
                    for(i = 255; i >= 0; i--)
                    {
                        if(list_next->this.vendor[i] == 0) continue;

                        else if(list_next->this.vendor[i] == 0x0A || list_next->this.vendor[i] == 0x0D ||
                                list_next->this.vendor[i] == ' ')
                            list_next->this.vendor[i] = 0;
                        else
                            break;
                    }
                }

                free(line_str);
                fclose(file);
            }
        }
        else if(strncmp(dirent->d_name, "loop", 4) == 0)
        {
            strncpy(list_next->this.vendor, "Linux", 256);
        }
        free((void*)tmp_string);

        tmp_string = malloc(1024);
        memset((void*)tmp_string, 0, 1024);
        snprintf((char*)tmp_string, 1024, "%s/%s/device/model", PATH_SYS_DEVBLOCK, dirent->d_name);

        if(access(tmp_string, R_OK) == 0 &&
           (strlen(list_next->this.model) == 0 || strncmp(list_next->this.bus, "ata", 3) == 0))
        {
            file = fopen(tmp_string, "rb");

            if(file != NULL)
            {
                line_str = malloc(256);
                memset(line_str, 0, 256);
                n   = 256;
                ret = getline(&line_str, &n, file);

                if(ret > 0 && line_str != NULL)
                {
                    strncpy(list_next->this.model, line_str, 256);
                    for(i = 255; i >= 0; i--)
                    {
                        if(list_next->this.model[i] == 0) continue;

                        else if(list_next->this.model[i] == 0x0A || list_next->this.model[i] == 0x0D ||
                                list_next->this.model[i] == ' ')
                            list_next->this.model[i] = 0;
                        else
                            break;
                    }
                }

                free(line_str);
                fclose(file);
            }
        }
        else if(strncmp(dirent->d_name, "loop", 4) == 0)
        {
            strncpy(list_next->this.model, "Linux", 256);
        }
        free((void*)tmp_string);

        tmp_string = malloc(1024);
        memset((void*)tmp_string, 0, 1024);
        snprintf((char*)tmp_string, 1024, "%s/%s/device/serial", PATH_SYS_DEVBLOCK, dirent->d_name);

        if(access(tmp_string, R_OK) == 0 && (strlen(list_next->this.serial) == 0))
        {
            file = fopen(tmp_string, "rb");

            if(file != NULL)
            {
                line_str = malloc(256);
                memset(line_str, 0, 256);
                n   = 256;
                ret = getline(&line_str, &n, file);

                if(ret > 0 && line_str != NULL)
                {
                    strncpy(list_next->this.serial, line_str, 256);
                    for(i = 255; i >= 0; i--)
                    {
                        if(list_next->this.serial[i] == 0) continue;

                        else if(list_next->this.serial[i] == 0x0A || list_next->this.serial[i] == 0x0D ||
                                list_next->this.serial[i] == ' ')
                            list_next->this.serial[i] = 0;
                        else
                            break;
                    }
                }

                free(line_str);
                fclose(file);
            }
        }
        free((void*)tmp_string);

        if(strlen(list_next->this.vendor) == 0 || strncmp(list_next->this.vendor, "ATA", 3) == 0)
        {
            if(strlen(list_next->this.model) > 0)
            {
                tmp_string = malloc(256);
                strncpy((void*)tmp_string, list_next->this.model, 256);

                chrptr = strchr(tmp_string, ' ');

                if(chrptr)
                {
                    memset(&list_next->this.vendor, 0, 256);
                    memset(&list_next->this.model, 0, 256);
                    strncpy(list_next->this.vendor, tmp_string, chrptr - tmp_string);
                    strncpy(list_next->this.model, chrptr + 1, 256 - (chrptr - tmp_string) - 1);
                }

                free((void*)tmp_string);
            }
        }

        // TODO: Get better device type from sysfs paths
        if(strlen(list_next->this.bus) == 0)
        {
            if(strncmp(dirent->d_name, "loop", 4) == 0) strncpy(list_next->this.bus, "loop", 4);
            else if(strncmp(dirent->d_name, "nvme", 4) == 0)
                strncpy(list_next->this.bus, "NVMe", 4);
            else if(strncmp(dirent->d_name, "mmc", 3) == 0)
                strncpy(list_next->this.bus, "MMC/SD", 6);
        }
        else
        {
            for(i = 0; i < 256; i++)
            {
                if(list_next->this.bus[i] == 0) break;

                list_next->this.bus[i] = (char)toupper(list_next->this.bus[i]);
            }
        }

        if(strncmp(list_next->this.bus, "ATA", 3) == 0 || strncmp(list_next->this.bus, "ATAPI", 5) == 0 ||
           strncmp(list_next->this.bus, "SCSI", 4) == 0 || strncmp(list_next->this.bus, "USB", 3) == 0 ||
           strncmp(list_next->this.bus, "PCMCIA", 6) == 0 || strncmp(list_next->this.bus, "FireWire", 8) == 0 ||
           strncmp(list_next->this.bus, "MMC/SD", 6) == 0)
            list_next->this.supported = true;
        else
            list_next->this.supported = false;

        list_current = list_next;
        dirent       = readdir(dir);
    }

    closedir(dir);

#ifdef HAS_UDEV
    if(has_udev) udev_unref(udev);
#endif

    return list_start;
}