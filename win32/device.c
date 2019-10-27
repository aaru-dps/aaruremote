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
    Win32DeviceContext* ctx = device_ctx;

    if(!ctx) return -1;

    // TODO: Implement
    return DICMOTE_DEVICE_TYPE_UNKNOWN;
}

int32_t Win32GetSdhciRegisters(void*     device_ctx,
                               char**    csd,
                               char**    cid,
                               char**    ocr,
                               char**    scr,
                               uint32_t* csd_len,
                               uint32_t* cid_len,
                               uint32_t* ocr_len,
                               uint32_t* scr_len)
{
    Win32DeviceContext* ctx = device_ctx;

    if(!ctx) return -1;

    // TODO: Implement
    return -1;
}