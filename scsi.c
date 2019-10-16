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
#endif

int32_t SendScsiCommand(int       device_fd,
                        char*     cdb,
                        char*     buffer,
                        char**    senseBuffer,
                        uint32_t  timeout,
                        int32_t   direction,
                        uint32_t* duration,
                        uint32_t* sense,
                        uint32_t  cdb_len,
                        uint32_t* buf_len,
                        uint32_t* sense_len)
{
#if defined(__linux__) && !defined(__ANDROID__)
    return linux_send_scsi_command(
        device_fd, cdb, buffer, senseBuffer, timeout, direction, duration, sense, cdb_len, buf_len, sense_len);
#else
    return -1;
#endif
}