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

#include <stdint.h>

#include <errno.h>
#include "mmc/ioctl.h"
#include <string.h>
#include <sys/ioctl.h>

int32_t LinuxSendSdhciCommand(int       device_fd,
                              uint8_t   command,
                              uint8_t   write,
                              uint8_t   application,
                              uint32_t  flags,
                              uint32_t  argument,
                              uint32_t  block_size,
                              uint32_t  blocks,
                              char*     buffer,
                              uint32_t  timeout,
                              uint32_t* response,
                              uint32_t* duration,
                              uint32_t* sense)
{
    struct mmc_ioc_cmd mmc_ioc_cmd;
    int32_t            error;
    *duration = 0;
    *sense    = 0;

    memset(response, 0, sizeof(uint32_t) * 4);
    memset(&mmc_ioc_cmd, 0, sizeof(struct mmc_ioc_cmd));

    mmc_ioc_cmd.write_flag = write;
    mmc_ioc_cmd.is_acmd    = application;
    mmc_ioc_cmd.opcode     = command;
    mmc_ioc_cmd.arg        = argument;
    mmc_ioc_cmd.flags      = flags;
    mmc_ioc_cmd.blksz      = block_size;
    mmc_ioc_cmd.blocks     = blocks;
    if(timeout > 0)
    {
        mmc_ioc_cmd.data_timeout_ns = timeout * 1000000000;
        mmc_ioc_cmd.cmd_timeout_ms  = timeout * 1000;
    }
    mmc_ioc_cmd.data_ptr = (uint64_t)buffer;

    // TODO: Timing
    error = ioctl(device_fd, MMC_IOC_CMD, &mmc_ioc_cmd);

    if(error < 0) error = errno;

    *sense = error < 0;

    memcpy((char*)response, (char*)&mmc_ioc_cmd.response, sizeof(uint32_t) * 4);

    return error;
}