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

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../aaruremote.h"
#include "freebsd.h"

void* DeviceOpen(const char* device_path)
{
    DeviceContext* ctx;

    ctx = malloc(sizeof(DeviceContext));

    if(!ctx) return NULL;

    memset(ctx, 0, sizeof(DeviceContext));

    ctx->fd = open(device_path, O_RDWR | O_NONBLOCK | O_CREAT);

    if((ctx->fd < 0) && (errno == EACCES || errno == EROFS)) ctx->fd = open(device_path, O_RDONLY | O_NONBLOCK);

    if(ctx->fd <= 0)
    {
        free(ctx);
        return NULL;
    }

    strncpy(ctx->device_path, device_path, 4096);

    ctx->device = cam_open_device(ctx->device_path, O_RDWR);

    if(!ctx->device)
    {
        close(ctx->fd);
        free(ctx);
        return NULL;
    }

    return ctx;
}

void DeviceClose(void* device_ctx)
{
    DeviceContext* ctx = device_ctx;

    if(!ctx) return;

    if(ctx->device)
        cam_close_device(ctx->device);

    close(ctx->fd);

    free(ctx);
}

int32_t GetDeviceType(void* device_ctx)
{
    return -1;
}
