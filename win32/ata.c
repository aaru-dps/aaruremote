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

#include <windows.h>

#include "win32.h"
#include "../aaruremote.h"

int32_t SendAtaChsCommand(void*                 device_ctx,
                          AtaRegistersChs       registers,
                          AtaErrorRegistersChs* error_registers,
                          uint8_t               protocol,
                          uint8_t               transfer_register,
                          char*                 buffer,
                          uint32_t              timeout,
                          uint8_t               transfer_blocks,
                          uint32_t*             duration,
                          uint32_t*             sense,
                          uint32_t*             buf_len)
{
    return -1;
}

int32_t SendAtaLba28Command(void*                   device_ctx,
                            AtaRegistersLba28       registers,
                            AtaErrorRegistersLba28* error_registers,
                            uint8_t                 protocol,
                            uint8_t                 transfer_register,
                            char*                   buffer,
                            uint32_t                timeout,
                            uint8_t                 transfer_blocks,
                            uint32_t*               duration,
                            uint32_t*               sense,
                            uint32_t*               buf_len)
{
    return -1;
}

int32_t SendAtaLba48Command(void*                   device_ctx,
                            AtaRegistersLba48       registers,
                            AtaErrorRegistersLba48* error_registers,
                            uint8_t                 protocol,
                            uint8_t                 transfer_register,
                            char*                   buffer,
                            uint32_t                timeout,
                            uint8_t                 transfer_blocks,
                            uint32_t*               duration,
                            uint32_t*               sense,
                            uint32_t*               buf_len)
{
    return -1;
}