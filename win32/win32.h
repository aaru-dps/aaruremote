/*
 * This file is part of the DiscImageChef Remote Server.
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

#ifndef DICREMOTE_WIN32_WIN32_H_
#define DICREMOTE_WIN32_WIN32_H_

#include "../dicmote.h"

#define PROCESSOR_ARCHITECTURE_INTEL 0
#define PROCESSOR_ARCHITECTURE_MIPS 1
#define PROCESSOR_ARCHITECTURE_ALPHA 2
#define PROCESSOR_ARCHITECTURE_PPC 3
#define PROCESSOR_ARCHITECTURE_SHX 4
#define PROCESSOR_ARCHITECTURE_ARM 5
#define PROCESSOR_ARCHITECTURE_IA64 6
#define PROCESSOR_ARCHITECTURE_ALPHA64 7
#define PROCESSOR_ARCHITECTURE_MSIL 8
#define PROCESSOR_ARCHITECTURE_AMD64 9
#define PROCESSOR_ARCHITECTURE_IA32_ON_WIN64 10
#define PROCESSOR_ARCHITECTURE_ARM64 12
#define PROCESSOR_ARCHITECTURE_ARM32_ON_WIN64 13
#define PROCESSOR_ARCHITECTURE_IA32_ON_ARM64 14

typedef struct
{
    SOCKET socket;
} Win32NetworkContext;

typedef struct
{
    HANDLE handle;
    char   device_path[4096];
} Win32DeviceContext;

DeviceInfoList* Win32ListDevices();
void*           Win32OpenDevice(const char* device_path);
void            Win32CloseDevice(void* device_ctx);

int32_t Win32GetDeviceType(void* device_ctx);

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
                             uint32_t* sense_len);

int32_t Win32GetSdhciRegisters(void*     device_ctx,
                               char**    csd,
                               char**    cid,
                               char**    ocr,
                               char**    scr,
                               uint32_t* csd_len,
                               uint32_t* cid_len,
                               uint32_t* ocr_len,
                               uint32_t* scr_len);

uint8_t Win32GetUsbData(void*     device_ctx,
                        uint16_t* desc_len,
                        char*     descriptors,
                        uint16_t* id_vendor,
                        uint16_t* id_product,
                        char*     manufacturer,
                        char*     product,
                        char*     serial);

uint8_t Win32GetIeee1394Data(void*     device_ctx,
                             uint32_t* id_model,
                             uint32_t* id_vendor,
                             uint64_t* guid,
                             char*     vendor,
                             char*     model);

uint8_t Win32GetPcmciaData(void* device_ctx, uint16_t* cis_len, char* cis);

int32_t Win32SendAtaChsCommand(void*                 device_ctx,
                               AtaRegistersChs       registers,
                               AtaErrorRegistersChs* error_registers,
                               uint8_t               protocol,
                               uint8_t               transfer_register,
                               char*                 buffer,
                               uint32_t              timeout,
                               uint8_t               transfer_blocks,
                               uint32_t*             duration,
                               uint32_t*             sense,
                               uint32_t*             buf_len);

int32_t Win32SendAtaLba28Command(void*                   device_ctx,
                                 AtaRegistersLba28       registers,
                                 AtaErrorRegistersLba28* error_registers,
                                 uint8_t                 protocol,
                                 uint8_t                 transfer_register,
                                 char*                   buffer,
                                 uint32_t                timeout,
                                 uint8_t                 transfer_blocks,
                                 uint32_t*               duration,
                                 uint32_t*               sense,
                                 uint32_t*               buf_len);

int32_t Win32SendAtaLba48Command(void*                   device_ctx,
                                 AtaRegistersLba48       registers,
                                 AtaErrorRegistersLba48* error_registers,
                                 uint8_t                 protocol,
                                 uint8_t                 transfer_register,
                                 char*                   buffer,
                                 uint32_t                timeout,
                                 uint8_t                 transfer_blocks,
                                 uint32_t*               duration,
                                 uint32_t*               sense,
                                 uint32_t*               buf_len);

int32_t Win32SendSdhciCommand(void*     device_ctx,
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
                              uint32_t* sense);

#endif // DICREMOTE_WIN32_WIN32_H_
