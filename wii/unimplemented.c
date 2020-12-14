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

// TODO

void* DeviceOpen(const char* device_path) { return NULL; }

void DeviceClose(void* device_ctx) {}

int32_t GetDeviceType(void* device_ctx) { return AARUREMOTE_DEVICE_TYPE_UNKNOWN; }

int32_t GetSdhciRegisters(void*     device_ctx,
                          char**    csd,
                          char**    cid,
                          char**    ocr,
                          char**    scr,
                          uint32_t* csd_len,
                          uint32_t* cid_len,
                          uint32_t* ocr_len,
                          uint32_t* scr_len)
{
    return 0;
}

int32_t SendSdhciCommand(void*     device_ctx,
                         uint8_t   command,
                         uint8_t   write,
                         uint8_t   application,
                         uint32_t  flags,
                         uint32_t  argument,
                         uint32_t  block_size,
                         uint32_t  blocks,
                         char*     buffer,
                         uint32_t  buf_len,
                         uint32_t  timeout,
                         uint32_t* response,
                         uint32_t* duration,
                         uint32_t* sense)
{
    return -1;
}

int32_t SendMultiSdhciCommand(void*            device_ctx,
                              uint64_t         count,
                              MmcSingleCommand commands[],
                              uint32_t*        duration,
                              uint32_t*        sense)
{
    return -1;
}
int32_t ReOpen(void* device_ctx, uint32_t* closeFailed)
{
    *closeFailed = 1;

    return -1;
}

int32_t OsRead(void* device_ctx, char* buffer, uint64_t offset, uint32_t length, uint32_t* duration) { return -1; }