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

#include <stdint.h>

int32_t Win32SendScsiCommand(void*     device_ctx,
                             char*     cdb,
                             char*     buffer,
                             char**    sense_buffer,
                             uint32_t  timeout,
                             int32_t   direction,
                             uint32_t* duration,
                             uint32_t* sense,
                             uint32_t  cdb_len,
                             uint32_t* buf_len,
                             uint32_t* sense_len)
{
    Win32DeviceContext* ctx = device_ctx;

    if(!ctx) return -1;

    // TODO: Implement
    return -1;
}