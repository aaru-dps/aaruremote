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

#include "dicmote.h"

#if defined(__linux__) && !defined(__ANDROID__)
#include "linux/linux.h"
#endif

int32_t SendAtaChsCommand(int                   device_fd,
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
#if defined(__linux__) && !defined(__ANDROID__)
    return LinuxSendAtaChsCommand(device_fd,
                                  registers,
                                  error_registers,
                                  protocol,
                                  transfer_register,
                                  buffer,
                                  timeout,
                                  transfer_blocks,
                                  duration,
                                  sense,
                                  buf_len);
#else
    return -1;
#endif
}

int32_t SendAtaLba28Command(int                     device_fd,
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
#if defined(__linux__) && !defined(__ANDROID__)
    return LinuxSendAtaLba28Command(device_fd,
                                    registers,
                                    error_registers,
                                    protocol,
                                    transfer_register,
                                    buffer,
                                    timeout,
                                    transfer_blocks,
                                    duration,
                                    sense,
                                    buf_len);
#else
    return -1;
#endif
}

int32_t SendAtaLba48Command(int                     device_fd,
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
#if defined(__linux__) && !defined(__ANDROID__)
    return LinuxSendAtaLba48Command(device_fd,
                                    registers,
                                    error_registers,
                                    protocol,
                                    transfer_register,
                                    buffer,
                                    timeout,
                                    transfer_blocks,
                                    duration,
                                    sense,
                                    buf_len);
#else
    return -1;
#endif
}
