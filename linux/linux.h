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

#ifndef DICREMOTE_LINUX_LINUX_H_
#define DICREMOTE_LINUX_LINUX_H_

#include "../dicmote.h"

#define PATH_SYS_DEVBLOCK "/sys/block"

DeviceInfoList* LinuxListDevices();

void* LinuxOpenDevice(const char* device_path);
void  LinuxCloseDevice(void* device_ctx);

int32_t LinuxGetDeviceType(void* device_ctx);

int32_t LinuxSendScsiCommand(void*     device_ctx,
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

int32_t LinuxGetSdhciRegisters(void*     device_ctx,
                               char**    csd,
                               char**    cid,
                               char**    ocr,
                               char**    scr,
                               uint32_t* csd_len,
                               uint32_t* cid_len,
                               uint32_t* ocr_len,
                               uint32_t* scr_len);

uint8_t LinuxGetUsbData(void*     device_ctx,
                        uint16_t* desc_len,
                        char*     descriptors,
                        uint16_t* id_vendor,
                        uint16_t* id_product,
                        char*     manufacturer,
                        char*     product,
                        char*     serial);

uint8_t LinuxGetIeee1394Data(void*     device_ctx,
                             uint32_t* id_model,
                             uint32_t* id_vendor,
                             uint64_t* guid,
                             char*     vendor,
                             char*     model);

uint8_t LinuxGetPcmciaData(void* device_ctx, uint16_t* cis_len, char* cis);

int32_t LinuxSendAtaChsCommand(void*                 device_ctx,
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

int32_t LinuxSendAtaLba28Command(void*                   device_ctx,
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

int32_t LinuxSendAtaLba48Command(void*                   device_ctx,
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

int32_t LinuxSendSdhciCommand(void*     device_ctx,
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

typedef struct
{
    int  fd;
    char device_path[4096];
} LinuxDeviceContext;

#endif // DICREMOTE_LINUX_LINUX_H_
