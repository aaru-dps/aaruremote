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

#include "../aaruremote.h"
#include "win32.h"

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

void* Win32OpenDevice(const char* device_path)
{
    Win32DeviceContext* ctx;

    ctx = malloc(sizeof(Win32DeviceContext));

    if(!ctx) return NULL;

    memset(ctx, 0, sizeof(Win32DeviceContext));

    ctx->handle = CreateFile(device_path,
                             GENERIC_READ | GENERIC_WRITE,
                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                             NULL,
                             OPEN_EXISTING,
                             FILE_ATTRIBUTE_NORMAL,
                             NULL);

    if(ctx->handle == INVALID_HANDLE_VALUE)
    {
        free(ctx);
        return NULL;
    }

    strncpy(ctx->device_path, device_path, 4096);

    return ctx;
}

void Win32CloseDevice(void* device_ctx)
{
    Win32DeviceContext* ctx = device_ctx;

    if(!ctx) return;

    CloseHandle(ctx->handle);

    free(ctx);
}

int32_t Win32GetDeviceType(void* device_ctx)
{
    Win32DeviceContext*        ctx = device_ctx;
    STORAGE_PROPERTY_QUERY     query;
    DWORD                      error = 0;
    BOOL                       ret;
    DWORD                      returned;
    PSTORAGE_DEVICE_DESCRIPTOR descriptor;
    char*                      buf;

    if(!ctx) return -1;

    buf = malloc(1000);

    if(!buf) return DICMOTE_DEVICE_TYPE_UNKNOWN;

    query.PropertyId = StorageDeviceProperty;
    query.QueryType  = PropertyStandardQuery;

    memset(buf, 0, 1000);

    ret = DeviceIoControl(
        ctx->handle, IOCTL_STORAGE_QUERY_PROPERTY, &query, sizeof(STORAGE_PROPERTY_QUERY), buf, 1000, &returned, NULL);

    if(!ret) error = GetLastError();

    if(!ret && error != 0)
    {
        free(buf);
        return DICMOTE_DEVICE_TYPE_UNKNOWN;
    }

    descriptor = (PSTORAGE_DEVICE_DESCRIPTOR)buf;

    switch(descriptor->BusType)
    {
        case 1: returned = DICMOTE_DEVICE_TYPE_SCSI; break;
        case 2: returned = DICMOTE_DEVICE_TYPE_ATAPI; break;
        case 3: returned = DICMOTE_DEVICE_TYPE_ATA; break;
        case 4: returned = DICMOTE_DEVICE_TYPE_SCSI; break;
        case 5: returned = DICMOTE_DEVICE_TYPE_SCSI; break;
        case 6: returned = DICMOTE_DEVICE_TYPE_SCSI; break;
        case 7: returned = DICMOTE_DEVICE_TYPE_SCSI; break;
        case 9: returned = DICMOTE_DEVICE_TYPE_SCSI; break;
        case 0xA: returned = DICMOTE_DEVICE_TYPE_SCSI; break;
        case 0xB: returned = DICMOTE_DEVICE_TYPE_ATA; break;
        case 0xC: returned = DICMOTE_DEVICE_TYPE_SECURE_DIGITAL; break;
        case 0xD: returned = DICMOTE_DEVICE_TYPE_MMC; break;
        case 0x11: returned = DICMOTE_DEVICE_TYPE_NVME; break;
        default: returned = DICMOTE_DEVICE_TYPE_UNKNOWN; break;
    }

    free(buf);

    return returned;
}
