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

#ifndef DICREMOTE_LINUX_H
#define DICREMOTE_LINUX_H

#include "../dicmote.h"

#define PATH_SYS_DEVBLOCK "/sys/block"
DeviceInfoList* linux_list_devices();
int             linux_open_device(const char* device_path);
int32_t         linux_get_device_type(const char* devicePath);
int32_t         linux_send_scsi_command(int       device_fd,
                                        char*     cdb,
                                        char*     buffer,
                                        char**    senseBuffer,
                                        uint32_t  timeout,
                                        int32_t   direction,
                                        uint32_t* duration,
                                        uint32_t* sense,
                                        uint32_t  cdb_len,
                                        uint32_t* buf_len,
                                        uint32_t* sense_len);
int32_t         linux_get_sdhci_registers(const char* devicePath,
                                          char**      csd,
                                          char**      cid,
                                          char**      ocr,
                                          char**      scr,
                                          uint32_t*   csd_len,
                                          uint32_t*   cid_len,
                                          uint32_t*   ocr_len,
                                          uint32_t*   scr_len);
uint8_t         linux_get_usb_data(const char* devicePath,
                                   uint16_t*   descLen,
                                   char*       descriptors,
                                   uint16_t*   idVendor,
                                   uint16_t*   idProduct,
                                   char*       manufacturer,
                                   char*       product,
                                   char*       serial);
uint8_t         linux_get_ieee1394_data(const char* devicePath,
                                        uint32_t*   idModel,
                                        uint32_t*   idVendor,
                                        uint64_t*   guid,
                                        char*       vendor,
                                        char*       model);
uint8_t         linux_get_pcmcia_data(const char* devicePath, uint16_t* cisLen, char* cis);
int32_t         linux_send_ata_chs_command(int                   device_fd,
                                           AtaRegistersChs       registers,
                                           AtaErrorRegistersChs* errorRegisters,
                                           uint8_t               protocol,
                                           uint8_t               transferRegister,
                                           char*                 buffer,
                                           uint32_t              timeout,
                                           uint8_t               transferBlocks,
                                           uint32_t*             duration,
                                           uint32_t*             sense,
                                           uint32_t*             buf_len);
int32_t         linux_send_ata_lba28_command(int                     device_fd,
                                             AtaRegistersLba28       registers,
                                             AtaErrorRegistersLba28* errorRegisters,
                                             uint8_t                 protocol,
                                             uint8_t                 transferRegister,
                                             char*                   buffer,
                                             uint32_t                timeout,
                                             uint8_t                 transferBlocks,
                                             uint32_t*               duration,
                                             uint32_t*               sense,
                                             uint32_t*               buf_len);
int32_t         linux_send_ata_lba48_command(int                     device_fd,
                                             AtaRegistersLba48       registers,
                                             AtaErrorRegistersLba48* errorRegisters,
                                             uint8_t                 protocol,
                                             uint8_t                 transferRegister,
                                             char*                   buffer,
                                             uint32_t                timeout,
                                             uint8_t                 transferBlocks,
                                             uint32_t*               duration,
                                             uint32_t*               sense,
                                             uint32_t*               buf_len);
int32_t         linux_send_sdhci_command(int       device_fd,
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
#endif // DICREMOTE_LINUX_H
