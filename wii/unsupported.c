/*
 * This file is part of the Aaru Remote Server.
 * Copyright (c) 2019-2021 Natalia Portillo.
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

uint8_t GetFireWireData(void*     device_ctx,
                        uint32_t* id_model,
                        uint32_t* id_vendor,
                        uint64_t* guid,
                        char*     vendor,
                        char*     model)
{
    return 0;
}

uint8_t GetPcmciaData(void* device_ctx, uint16_t* cis_len, char* cis) { return 0; }

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
    return -1;
}

uint8_t GetUsbData(void*     device_ctx,
                   uint16_t* desc_len,
                   char*     descriptors,
                   uint16_t* id_vendor,
                   uint16_t* id_product,
                   char*     manufacturer,
                   char*     product,
                   char*     serial)
{
    return 0;
}
