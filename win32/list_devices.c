/*
 * This file is part of the DiscImageChef Remote Server.
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

#include "../dicmote.h"

#include <stddef.h>
#include <stdio.h>
#include <windows.h>

#ifndef IOCTL_STORAGE_QUERY_PROPERTY
#define IOCTL_STORAGE_QUERY_PROPERTY 0x2D1400
#endif

DeviceInfoList* Win32ListDevices()
{
    char                       physId[4096];
    LPSTR                      physical;
    HANDLE                     handle;
    STORAGE_PROPERTY_QUERY     query;
    DWORD                      error = 0;
    BOOL                       ret;
    DWORD                      returned;
    PSTORAGE_DEVICE_DESCRIPTOR descriptor;
    char*                      buf;
    DeviceInfoList *           list_start = NULL, *list_current = NULL, *list_next = NULL;
    LPSTR                      chr;
    LPSTR                      tmpstring;
    LPSTR                      pos;

    buf = malloc(1000);

    if(!buf) return NULL;

    physical = malloc(65536);

    if(!physical)
    {
        free(buf);
        return NULL;
    }

    QueryDosDevice(NULL, physical, 65536);

    for(pos = physical; *pos; pos += strlen(pos) + 1)
    {
        if((strncmp(pos, "PhysicalDrive", 13) != 0 && strncmp(pos, "CdRom", 5) != 0) && strncmp(pos, "Tape", 4) != 0)
        { continue; }

        snprintf(physId, 4096, "\\\\.\\%s", pos);

        printf("%s\n", physId);

        handle = CreateFile(physId, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

        if(handle == INVALID_HANDLE_VALUE) { continue; }

        query.PropertyId = StorageDeviceProperty;
        query.QueryType  = PropertyStandardQuery;

        memset(buf, 0, 1000);

        ret = DeviceIoControl(
            handle, IOCTL_STORAGE_QUERY_PROPERTY, &query, sizeof(STORAGE_PROPERTY_QUERY), buf, 1000, &returned, NULL);

        if(!ret) error = GetLastError();

        if(!ret && error != 0)
        {
            CloseHandle(handle);
            continue;
        }

        descriptor = (PSTORAGE_DEVICE_DESCRIPTOR)buf;

        list_next = malloc(sizeof(DeviceInfoList));
        memset(list_next, 0, sizeof(DeviceInfoList));

        if(!list_next)
        {
            CloseHandle(handle);
            continue;
        }

        if(!list_start) list_start = list_next;

        if(list_current) list_current->next = list_next;

        strncpy(list_next->this.path, physId, 1024);

        switch(descriptor->BusType)
        {
            case 1: strncpy(list_next->this.bus, "SCSI", 4); break;
            case 2: strncpy(list_next->this.bus, "ATAPI", 5); break;
            case 3: strncpy(list_next->this.bus, "ATA", 3); break;
            case 4: strncpy(list_next->this.bus, "FireWire", 8); break;
            case 5: strncpy(list_next->this.bus, "SSA", 3); break;
            case 6: strncpy(list_next->this.bus, "Fibre", 5); break;
            case 7: strncpy(list_next->this.bus, "USB", 3); break;
            case 8: strncpy(list_next->this.bus, "RAID", 4); break;
            case 9: strncpy(list_next->this.bus, "iSCSI", 5); break;
            case 0xA: strncpy(list_next->this.bus, "SAS", 3); break;
            case 0xB: strncpy(list_next->this.bus, "SATA", 4); break;
            case 0xC: strncpy(list_next->this.bus, "SecureDigital", 13); break;
            case 0xD: strncpy(list_next->this.bus, "MultiMediaCard", 14); break;
            case 0xE: strncpy(list_next->this.bus, "Virtual", 7); break;
            case 0xF: strncpy(list_next->this.bus, "FileBackedVirtual", 17); break;
            case 0x11: strncpy(list_next->this.bus, "NVMe", 4); break;
            case 16: strncpy(list_next->this.bus, "Spaces", 6); break;
            case 18: strncpy(list_next->this.bus, "SCM", 3); break;
            case 19: strncpy(list_next->this.bus, "UFS", 3); break;
            default: strncpy(list_next->this.bus, "Unknown", 4); break;
        }

        switch(descriptor->BusType)
        {
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            case 9:
            case 0xA:
            case 0xB:
            case 0xC:
            case 0xD: list_next->this.supported = TRUE; break;
            default: list_next->this.supported = FALSE; break;
        }

        if(descriptor->VendorIdOffset > 0) strncpy(list_next->this.vendor, buf + descriptor->VendorIdOffset, 256);

        if(descriptor->ProductIdOffset > 0) strncpy(list_next->this.model, buf + descriptor->ProductIdOffset, 256);

        // TODO: Get serial number of SCSI and USB devices, probably also FireWire (untested)
        if(descriptor->SerialNumberOffset > 0)
        {
            strncpy(list_next->this.serial, buf + descriptor->SerialNumberOffset, 256);

            // TODO: fix any serial numbers that are returned as hex-strings
        }

        if((strlen(list_next->this.vendor) == 0 || strncmp(list_next->this.vendor, "ATA", 3) == 0) &&
           strlen(list_next->this.model) > 0)
        {
            tmpstring = malloc(256);
            if(tmpstring)
            {
                strncpy(tmpstring, list_next->this.model, 256);
                chr = strstr(tmpstring, " ");

                if(chr)
                {
                    memset(&list_next->this.model, 0, 256);
                    memset(&list_next->this.vendor, 0, 256);
                    strncpy(list_next->this.vendor, tmpstring, chr - tmpstring);
                    strncpy(list_next->this.model, chr + 1, 256 - strlen(list_next->this.vendor));
                }

                free(tmpstring);
            }
        }

        CloseHandle(handle);
        list_current = list_next;
    }

    return list_start;
}
