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

#include <stdint.h>

#include "../aaruremote.h"
#include "freebsd.h"

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
    return 0;
}

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