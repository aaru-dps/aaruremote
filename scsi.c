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

#if defined(__linux__) && !defined(__ANDROID__)
#include "linux/linux.h"
#elif defined(WIN32)
#include "win32/win32.h"
#endif

#include <stdint.h>

int32_t SendScsiCommand(void*     device_ctx,
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
#if defined(__linux__) && !defined(__ANDROID__)
    return LinuxSendScsiCommand(
        device_ctx, cdb, buffer, sense_buffer, timeout, direction, duration, sense, cdb_len, buf_len, sense_len);
#elif defined(WIN32)
    return Win32SendScsiCommand(
        device_ctx, cdb, buffer, sense_buffer, timeout, direction, duration, sense, cdb_len, buf_len, sense_len);
#else
    return -1;
#endif
}