/*
 * This file is part of the Aaru Remote Server.
 * Copyright (c) 2019-2026 Natalia Portillo.
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
#include <stdint.h>
#include <string.h>

#include "../aaruremote.h"

int32_t SendSdhciCommand(void *device_ctx, uint8_t command, uint8_t write, uint8_t application, uint32_t flags,
                         uint32_t argument, uint32_t block_size, uint32_t blocks, char *buffer, uint32_t buf_len,
                         uint32_t timeout, uint32_t *response, uint32_t *duration, uint32_t *sense)
{
    (void)device_ctx;
    (void)command;
    (void)write;
    (void)application;
    (void)flags;
    (void)argument;
    (void)block_size;
    (void)blocks;
    (void)buffer;
    (void)buf_len;
    (void)timeout;

    if(response) memset(response, 0, sizeof(uint32_t) * 4);
    if(duration) *duration = 0;
    if(sense) *sense = 1;

    return ENOSYS;
}

int32_t SendMultiSdhciCommand(void *device_ctx, uint64_t count, MmcSingleCommand commands[], uint32_t *duration,
                              uint32_t *sense)
{
    (void)device_ctx;
    (void)count;
    (void)commands;

    if(duration) *duration = 0;
    if(sense) *sense = 1;

    return ENOSYS;
}

int32_t GetSdhciRegisters(void *device_ctx, char **csd, char **cid, char **ocr, char **scr, uint32_t *csd_len,
                          uint32_t *cid_len, uint32_t *ocr_len, uint32_t *scr_len)
{
    (void)device_ctx;

    if(csd) *csd = NULL;
    if(cid) *cid = NULL;
    if(ocr) *ocr = NULL;
    if(scr) *scr = NULL;
    if(csd_len) *csd_len = 0;
    if(cid_len) *cid_len = 0;
    if(ocr_len) *ocr_len = 0;
    if(scr_len) *scr_len = 0;

    return 0;
}
