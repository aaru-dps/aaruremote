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

#include "../dicmote.h"
#include "linux.h"

#include <ctype.h>
#include <dirent.h>
#include <libudev.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

DeviceInfoList* linux_list_devices()
{
    DIR*                dir;
    struct dirent*      dirent;
    struct udev*        udev;
    int                 hasUdev   = false, i;
    DeviceInfoList *    listStart = NULL, *listCurrent = NULL, *listNext = NULL;
    struct udev_device* udev_device;
    const char*         tmpString;
    FILE*               file;
    char*               line_str;
    size_t              n, ret;
    char*               chrptr;

    udev = udev_new();

    hasUdev = udev != 0;

    dir = opendir(PATH_SYS_DEVBLOCK);
    if(!dir) return NULL;

    dirent = readdir(dir);

    while(dirent)
    {
        if(dirent->d_type != DT_REG && dirent->d_type != DT_LNK)
        {
            dirent = readdir(dir);
            continue;
        }

        listNext = malloc(sizeof(DeviceInfoList));
        memset(listNext, 0, sizeof(DeviceInfoList));

        if(!listNext)
        {
            closedir(dir);

            if(hasUdev) udev_unref(udev);

            return listStart;
        }

        if(!listStart) listStart = listNext;

        if(listCurrent) listCurrent->next = listNext;

        snprintf(listNext->this.path, 1024, "/dev/%s", dirent->d_name);

        if(hasUdev)
        {
            udev_device = udev_device_new_from_subsystem_sysname(udev, "block", dirent->d_name);
            if(udev_device)
            {
                tmpString = udev_device_get_property_value(udev_device, "ID_VENDOR");
                if(tmpString)
                {
                    strncpy(listNext->this.vendor, tmpString, 256);
                    free((void*)tmpString);
                }

                tmpString = udev_device_get_property_value(udev_device, "ID_MODEL");
                if(tmpString)
                {
                    strncpy(listNext->this.model, tmpString, 256);
                    free((void*)tmpString);

                    for(i = 0; i < 256; i++)
                    {
                        if(listNext->this.model[i] == 0) break;

                        if(listNext->this.model[i] == '_') listNext->this.model[i] = ' ';
                    }
                }

                tmpString = udev_device_get_property_value(udev_device, "ID_SCSI_SERIAL");
                if(tmpString)
                {
                    strncpy(listNext->this.serial, tmpString, 256);
                    free((void*)tmpString);
                }
                else
                {
                    tmpString = udev_device_get_property_value(udev_device, "ID_SERIAL_SHORT");
                    if(tmpString)
                    {
                        strncpy(listNext->this.serial, tmpString, 256);
                        free((void*)tmpString);
                    }
                }

                tmpString = udev_device_get_property_value(udev_device, "ID_BUS");
                if(tmpString)
                {
                    strncpy(listNext->this.bus, tmpString, 256);
                    free((void*)tmpString);
                }
            }
        }

        tmpString = malloc(1024);
        memset((void*)tmpString, 0, 1024);
        snprintf((char*)tmpString, 1024, "%s/%s/device/vendor", PATH_SYS_DEVBLOCK, dirent->d_name);

        if(access(tmpString, R_OK) == 0 && strlen(listNext->this.vendor) == 0)
        {
            file = fopen(tmpString, "rb");

            if(file != NULL)
            {
                line_str = malloc(256);
                memset(line_str, 0, 256);
                n   = 256;
                ret = getline(&line_str, &n, file);

                if(ret > 0 && line_str != NULL)
                {
                    strncpy(listNext->this.vendor, line_str, 256);
                    for(i = 255; i >= 0; i--)
                    {
                        if(listNext->this.vendor[i] == 0)
                            continue;

                        else if(listNext->this.vendor[i] == 0x0A || listNext->this.vendor[i] == 0x0D ||
                                listNext->this.vendor[i] == ' ')
                            listNext->this.vendor[i] = 0;
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
            strncpy(listNext->this.vendor, "Linux", 256);
        }
        free((void*)tmpString);

        tmpString = malloc(1024);
        memset((void*)tmpString, 0, 1024);
        snprintf((char*)tmpString, 1024, "%s/%s/device/model", PATH_SYS_DEVBLOCK, dirent->d_name);

        if(access(tmpString, R_OK) == 0 &&
           (strlen(listNext->this.model) == 0 || strncmp(listNext->this.bus, "ata", 3) == 0))
        {
            file = fopen(tmpString, "rb");

            if(file != NULL)
            {
                line_str = malloc(256);
                memset(line_str, 0, 256);
                n   = 256;
                ret = getline(&line_str, &n, file);

                if(ret > 0 && line_str != NULL)
                {
                    strncpy(listNext->this.model, line_str, 256);
                    for(i = 255; i >= 0; i--)
                    {
                        if(listNext->this.model[i] == 0)
                            continue;

                        else if(listNext->this.model[i] == 0x0A || listNext->this.model[i] == 0x0D ||
                                listNext->this.model[i] == ' ')
                            listNext->this.model[i] = 0;
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
            strncpy(listNext->this.model, "Linux", 256);
        }
        free((void*)tmpString);

        tmpString = malloc(1024);
        memset((void*)tmpString, 0, 1024);
        snprintf((char*)tmpString, 1024, "%s/%s/device/serial", PATH_SYS_DEVBLOCK, dirent->d_name);

        if(access(tmpString, R_OK) == 0 && (strlen(listNext->this.serial) == 0))
        {
            file = fopen(tmpString, "rb");

            if(file != NULL)
            {
                line_str = malloc(256);
                memset(line_str, 0, 256);
                n   = 256;
                ret = getline(&line_str, &n, file);

                if(ret > 0 && line_str != NULL)
                {
                    strncpy(listNext->this.serial, line_str, 256);
                    for(i = 255; i >= 0; i--)
                    {
                        if(listNext->this.serial[i] == 0)
                            continue;

                        else if(listNext->this.serial[i] == 0x0A || listNext->this.serial[i] == 0x0D ||
                                listNext->this.serial[i] == ' ')
                            listNext->this.serial[i] = 0;
                        else
                            break;
                    }
                }

                free(line_str);
                fclose(file);
            }
        }
        free((void*)tmpString);

        if(strlen(listNext->this.vendor) == 0 || strncmp(listNext->this.vendor, "ATA", 3) == 0)
        {
            if(strlen(listNext->this.model) > 0)
            {
                tmpString = malloc(256);
                strncpy((void*)tmpString, listNext->this.model, 256);

                chrptr = strchr(tmpString, ' ');

                if(chrptr)
                {
                    memset(&listNext->this.vendor, 0, 256);
                    memset(&listNext->this.model, 0, 256);
                    strncpy(listNext->this.vendor, tmpString, chrptr - tmpString);
                    strncpy(listNext->this.model, chrptr + 1, 256 - (chrptr - tmpString) - 1);
                }

                free((void*)tmpString);
            }
        }

        // TODO: Get better device type from sysfs paths
        if(strlen(listNext->this.bus) == 0)
        {
            if(strncmp(dirent->d_name, "loop", 4) == 0)
                strncpy(listNext->this.bus, "loop", 4);
            else if(strncmp(dirent->d_name, "nvme", 4) == 0)
                strncpy(listNext->this.bus, "NVMe", 4);
            else if(strncmp(dirent->d_name, "mmc", 3) == 0)
                strncpy(listNext->this.bus, "MMC/SD", 6);
        }
        else
        {
            for(i = 0; i < 256; i++)
            {
                if(listNext->this.bus[i] == 0) break;

                listNext->this.bus[i] = (char)toupper(listNext->this.bus[i]);
            }
        }

        if(strncmp(listNext->this.bus, "ATA", 3) == 0 || strncmp(listNext->this.bus, "ATAPI", 5) == 0 ||
           strncmp(listNext->this.bus, "SCSI", 4) == 0 || strncmp(listNext->this.bus, "USB", 3) == 0 ||
           strncmp(listNext->this.bus, "PCMCIA", 6) == 0 || strncmp(listNext->this.bus, "FireWire", 8) == 0 ||
           strncmp(listNext->this.bus, "MMC/SD", 6) == 0)
            listNext->this.supported = true;
        else
            listNext->this.supported = false;

        listCurrent = listNext;
        dirent      = readdir(dir);
    }

    closedir(dir);

    if(hasUdev) udev_unref(udev);

    return listStart;
}