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

int32_t SendSdhciCommand(int       device_fd,
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
#if defined(__linux__) && !defined(__ANDROID__)
    return LinuxSendSdhciCommand(device_fd,
                                 command,
                                 write,
                                 application,
                                 flags,
                                 argument,
                                 block_size,
                                 blocks,
                                 buffer,
                                 timeout,
                                 response,
                                 duration,
                                 sense);
#else
    return -1;
#endif
}