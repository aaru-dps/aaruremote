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

#ifndef AARUREMOTE__AARUREMOTE_H_
#define AARUREMOTE__AARUREMOTE_H_

#ifdef GEKKO
#include <network.h>
#ifndef MSG_PEEK
#define MSG_PEEK 0x02 // TODO: Untested, may not work
#endif
#elif !defined(_WIN32)
#include <netinet/in.h>
#include <sys/socket.h>
#endif

#define AARUREMOTE_NAME "Aaru Remote Server"
#define AARUREMOTE_VERSION "0.99.195"
#define AARUREMOTE_PORT 6666
#define AARUREMOTE_REMOTE_ID 0x52434944 // "DICR"
#define AARUREMOTE_PACKET_ID 0x544B4350 // "PCKT"
#define AARUREMOTE_PACKET_VERSION 1
#define AARUREMOTE_PACKET_TYPE_NOP -1
#define AARUREMOTE_PACKET_TYPE_HELLO 1
#define AARUREMOTE_PACKET_TYPE_COMMAND_LIST_DEVICES 2
#define AARUREMOTE_PACKET_TYPE_RESPONSE_LIST_DEVICES 3
#define AARUREMOTE_PACKET_TYPE_COMMAND_OPEN_DEVICE 4
#define AARUREMOTE_PACKET_TYPE_COMMAND_SCSI 5
#define AARUREMOTE_PACKET_TYPE_RESPONSE_SCSI 6
#define AARUREMOTE_PACKET_TYPE_COMMAND_ATA_CHS 7
#define AARUREMOTE_PACKET_TYPE_RESPONSE_ATA_CHS 8
#define AARUREMOTE_PACKET_TYPE_COMMAND_ATA_LBA_28 9
#define AARUREMOTE_PACKET_TYPE_RESPONSE_ATA_LBA_28 10
#define AARUREMOTE_PACKET_TYPE_COMMAND_ATA_LBA_48 11
#define AARUREMOTE_PACKET_TYPE_RESPONSE_ATA_LBA_48 12
#define AARUREMOTE_PACKET_TYPE_COMMAND_SDHCI 13
#define AARUREMOTE_PACKET_TYPE_RESPONSE_SDHCI 14
#define AARUREMOTE_PACKET_TYPE_COMMAND_GET_DEVTYPE 15
#define AARUREMOTE_PACKET_TYPE_RESPONSE_GET_DEVTYPE 16
#define AARUREMOTE_PACKET_TYPE_COMMAND_GET_SDHCI_REGISTERS 17
#define AARUREMOTE_PACKET_TYPE_RESPONSE_GET_SDHCI_REGISTERS 18
#define AARUREMOTE_PACKET_TYPE_COMMAND_GET_USB_DATA 19
#define AARUREMOTE_PACKET_TYPE_RESPONSE_GET_USB_DATA 20
#define AARUREMOTE_PACKET_TYPE_COMMAND_GET_FIREWIRE_DATA 21
#define AARUREMOTE_PACKET_TYPE_RESPONSE_GET_FIREWIRE_DATA 22
#define AARUREMOTE_PACKET_TYPE_COMMAND_GET_PCMCIA_DATA 23
#define AARUREMOTE_PACKET_TYPE_RESPONSE_GET_PCMCIA_DATA 24
#define AARUREMOTE_PACKET_TYPE_COMMAND_CLOSE_DEVICE 25
#define AARUREMOTE_PACKET_TYPE_COMMAND_AM_I_ROOT 26
#define AARUREMOTE_PACKET_TYPE_RESPONSE_AM_I_ROOT 27
#define AARUREMOTE_PACKET_TYPE_MULTI_COMMAND_SDHCI 28
#define AARUREMOTE_PACKET_TYPE_RESPONSE_MULTI_SDHCI 29
#define AARUREMOTE_PACKET_TYPE_COMMAND_REOPEN 30
#define AARUREMOTE_PACKET_TYPE_COMMAND_OSREAD 31
#define AARUREMOTE_PACKET_TYPE_RESPONSE_OSREAD 32
#define AARUREMOTE_PROTOCOL_MAX 2
#define AARUREMOTE_PACKET_NOP_REASON_OOO 0
#define AARUREMOTE_PACKET_NOP_REASON_NOT_IMPLEMENTED 1
#define AARUREMOTE_PACKET_NOP_REASON_NOT_RECOGNIZED 2
#define AARUREMOTE_PACKET_NOP_REASON_ERROR_LIST_DEVICES 3
#define AARUREMOTE_PACKET_NOP_REASON_OPEN_OK 4
#define AARUREMOTE_PACKET_NOP_REASON_OPEN_ERROR 5
#define AARUREMOTE_PACKET_NOP_REASON_REOPEN_OK 6
#define AARUREMOTE_PACKET_NOP_REASON_CLOSE_ERROR 5
#define AARUREMOTE_DEVICE_TYPE_UNKNOWN -1
#define AARUREMOTE_DEVICE_TYPE_ATA 1
#define AARUREMOTE_DEVICE_TYPE_ATAPI 2
#define AARUREMOTE_DEVICE_TYPE_SCSI 3
#define AARUREMOTE_DEVICE_TYPE_SECURE_DIGITAL 4
#define AARUREMOTE_DEVICE_TYPE_MMC 5
#define AARUREMOTE_DEVICE_TYPE_NVME 6
#define AARUREMOTE_SCSI_DIRECTION_UNSPECIFIED -1
#define AARUREMOTE_SCSI_DIRECTION_NONE 0
#define AARUREMOTE_SCSI_DIRECTION_OUT 1
#define AARUREMOTE_SCSI_DIRECTION_IN 2
#define AARUREMOTE_SCSI_DIRECTION_INOUT 3
#define AARUREMOTE_ATA_PROTOCOL_HARD_RESET 0
#define AARUREMOTE_ATA_PROTOCOL_SOFT_RESET 1
#define AARUREMOTE_ATA_PROTOCOL_NO_DATA 3
#define AARUREMOTE_ATA_PROTOCOL_PIO_IN 4
#define AARUREMOTE_ATA_PROTOCOL_PIO_OUT 5
#define AARUREMOTE_ATA_PROTOCOL_DMA 6
#define AARUREMOTE_ATA_PROTOCOL_DMA_QUEUED 7
#define AARUREMOTE_ATA_PROTOCOL_DEVICE_DIAGNOSTIC 8
#define AARUREMOTE_ATA_PROTOCOL_DEVICE_RESET 9
#define AARUREMOTE_ATA_PROTOCOL_UDMA_IN 10
#define AARUREMOTE_ATA_PROTOCOL_UDMA_OUT 11
#define AARUREMOTE_ATA_PROTOCOL_FPDMA 12
#define AARUREMOTE_ATA_PROTOCOL_RETURN_RESPONSE 15
#define AARUREMOTE_ATA_TRANSFER_REGISTER_NONE 0
#define AARUREMOTE_ATA_TRANSFER_REGISTER_FEATURE 1
#define AARUREMOTE_ATA_TRANSFER_REGISTER_SECTOR_COUNT 2
#define AARUREMOTE_ATA_TRANSFER_REGISTER_SPTSIU 3
#define AARUREMOTE_MMC_RESPONSE_PRESENT (1 << 0)
#define AARUREMOTE_MMC_RESPONSE_136 (1 << 1)
#define AARUREMOTE_MMC_RESPONSE_CRC (1 << 2)
#define AARUREMOTE_MMC_RESPONSE_BUSY (1 << 3)
#define AARUREMOTE_MMC_RESPONSE_OPCODE (1 << 4)
#define AARUREMOTE_MMC_COMMAND_MASK (3 << 5)
#define AARUREMOTE_MMC_COMMAND_AC (0 << 5)
#define AARUREMOTE_MMC_COMMAND_ADTC (1 << 5)
#define AARUREMOTE_MMC_COMMAND_BC (2 << 5)
#define AARUREMOTE_MMC_COMMAND_BCR (3 << 5)
#define AARUREMOTE_MMC_RESPONSE_SPI_S1 (1 << 7)
#define AARUREMOTE_MMC_RESPONSE_SPI_S2 (1 << 8)
#define AARUREMOTE_MMC_RESPONSE_SPI_B4 (1 << 9)
#define AARUREMOTE_MMC_RESPONSE_SPI_BUSY (1 << 10)
#define AARUREMOTE_MMC_RESPONSE_NONE (0)
#define AARUREMOTE_MMC_RESPONSE_R1                                                                                     \
    AARUREMOTE_MMC_RESPONSE_PRESENT | AARUREMOTE_MMC_RESPONSE_CRC | AARUREMOTE_MMC_RESPONSE_OPCODE
#define AARUREMOTE_MMC_RESPONSE_R1B                                                                                    \
    AARUREMOTE_MMC_RESPONSE_PRESENT | AARUREMOTE_MMC_RESPONSE_CRC | AARUREMOTE_MMC_RESPONSE_OPCODE |                   \
        AARUREMOTE_MMC_RESPONSE_BUSY
#define AARUREMOTE_MMC_RESPONSE_R2                                                                                     \
    AARUREMOTE_MMC_RESPONSE_PRESENT | AARUREMOTE_MMC_RESPONSE_136 | AARUREMOTE_MMC_RESPONSE_CRC
#define AARUREMOTE_MMC_RESPONSE_R3 AARUREMOTE_MMC_RESPONSE_PRESENT
#define AARUREMOTE_MMC_RESPONSE_R4 AARUREMOTE_MMC_RESPONSE_PRESENT
#define AARUREMOTE_MMC_RESPONSE_R5                                                                                     \
    AARUREMOTE_MMC_RESPONSE_PRESENT | AARUREMOTE_MMC_RESPONSE_CRC | AARUREMOTE_MMC_RESPONSE_OPCODE
#define AARUREMOTE_MMC_RESPONSE_R6                                                                                     \
    AARUREMOTE_MMC_RESPONSE_PRESENT | AARUREMOTE_MMC_RESPONSE_CRC | AARUREMOTE_MMC_RESPONSE_OPCODE
#define AARUREMOTE_MMC_RESPONSE_R7                                                                                     \
    AARUREMOTE_MMC_RESPONSE_PRESENT | AARUREMOTE_MMC_RESPONSE_CRC | AARUREMOTE_MMC_RESPONSE_OPCODE
#define AARUREMOTE_MMC_RESPONSE_SPI_R1 AARUREMOTE_MMC_RESPONSE_SPI_S1
#define AARUREMOTE_MMC_RESPONSE_SPI_R1B AARUREMOTE_MMC_RESPONSE_SPI_S1 | AARUREMOTE_MMC_RESPONSE_SPI_BUSY
#define AARUREMOTE_MMC_RESPONSE_SPI_R2 AARUREMOTE_MMC_RESPONSE_SPI_S1 | AARUREMOTE_MMC_RESPONSE_SPI_S2
#define AARUREMOTE_MMC_RESPONSE_SPI_R3 AARUREMOTE_MMC_RESPONSE_SPI_S1 | AARUREMOTE_MMC_RESPONSE_SPI_B4
#define AARUREMOTE_MMC_RESPONSE_SPI_R4 AARUREMOTE_MMC_RESPONSE_SPI_S1 | AARUREMOTE_MMC_RESPONSE_SPI_B4
#define AARUREMOTE_MMC_RESPONSE_SPI_R5 AARUREMOTE_MMC_RESPONSE_SPI_S1 | AARUREMOTE_MMC_RESPONSE_SPI_S2
#define AARUREMOTE_MMC_RESPONSE_SPI_R7 AARUREMOTE_MMC_RESPONSE_SPI_S1 | AARUREMOTE_MMC_RESPONSE_SPI_B4

#pragma pack(push, 1)

typedef struct
{
    uint32_t remote_id;
    uint32_t packet_id;
    uint32_t len;
    uint8_t  version;
    int8_t   packet_type;
    char     spare[2];
} AaruPacketHeader;

typedef struct
{
    AaruPacketHeader hdr;
    char             application[128];
    char             version[64];
    uint8_t          max_protocol;
    char             spare[3];
    char             sysname[256];
    char             release[256];
    char             machine[256];
} AaruPacketHello;

typedef struct
{
    AaruPacketHeader hdr;
} AaruPacketCmdListDevs;

typedef struct
{
    AaruPacketHeader hdr;
    uint16_t         devices;
} AaruPacketResListDevs;

typedef struct
{
    char    path[1024];
    char    vendor[256];
    char    model[256];
    char    serial[256];
    char    bus[256];
    uint8_t supported;
    char    padding[3];
} DeviceInfo;

typedef struct DeviceInfoList
{
    struct DeviceInfoList* next;
    DeviceInfo this;
} DeviceInfoList;

typedef struct
{
    AaruPacketHeader hdr;
    uint8_t          reason_code;
    char             spare[3];
    char             reason[256];
    int32_t          error_no;
} AaruPacketNop;

typedef struct
{
    AaruPacketHeader hdr;
    char             device_path[1024];
} AaruPacketCmdOpen;

typedef struct
{
    AaruPacketHeader hdr;
    uint32_t         cdb_len;
    uint32_t         buf_len;
    int32_t          direction;
    uint32_t         timeout;
} AaruPacketCmdScsi;

typedef struct
{
    AaruPacketHeader hdr;
    uint32_t         sense_len;
    uint32_t         buf_len;
    uint32_t         duration;
    uint32_t         sense;
    uint32_t         error_no;
} AaruPacketResScsi;

typedef struct
{
    uint8_t feature;
    uint8_t sector_count;
    uint8_t sector;
    uint8_t cylinder_low;
    uint8_t cylinder_high;
    uint8_t device_head;
    uint8_t command;
} AtaRegistersChs;

typedef struct
{
    uint8_t status;
    uint8_t error;
    uint8_t sector_count;
    uint8_t sector;
    uint8_t cylinder_low;
    uint8_t cylinder_high;
    uint8_t device_head;
} AtaErrorRegistersChs;

typedef struct
{
    AaruPacketHeader hdr;
    uint32_t         buf_len;
    AtaRegistersChs  registers;
    uint8_t          protocol;
    uint8_t          transfer_register;
    uint8_t          transfer_blocks;
    uint8_t          spare;
    uint32_t         timeout;
} AaruPacketCmdAtaChs;

typedef struct
{
    AaruPacketHeader     hdr;
    uint32_t             buf_len;
    AtaErrorRegistersChs registers;
    uint32_t             duration;
    uint32_t             sense;
    uint32_t             error_no;
} AaruPacketResAtaChs;

typedef struct
{
    uint8_t feature;
    uint8_t sector_count;
    uint8_t lba_low;
    uint8_t lba_mid;
    uint8_t lba_high;
    uint8_t device_head;
    uint8_t command;
} AtaRegistersLba28;

typedef struct
{
    uint8_t status;
    uint8_t error;
    uint8_t sector_count;
    uint8_t lba_low;
    uint8_t lba_mid;
    uint8_t lba_high;
    uint8_t device_head;
} AtaErrorRegistersLba28;

typedef struct
{
    AaruPacketHeader  hdr;
    uint32_t          buf_len;
    AtaRegistersLba28 registers;
    uint8_t           protocol;
    uint8_t           transfer_register;
    uint8_t           transfer_blocks;
    uint8_t           spare;
    uint32_t          timeout;
} AaruPacketCmdAtaLba28;

typedef struct
{
    AaruPacketHeader       hdr;
    uint32_t               buf_len;
    AtaErrorRegistersLba28 registers;
    uint32_t               duration;
    uint32_t               sense;
    uint32_t               error_no;
} AaruPacketResAtaLba28;

typedef struct
{
    uint16_t feature;
    uint16_t sector_count;
    uint16_t lba_low;
    uint16_t lba_mid;
    uint16_t lba_high;
    uint8_t  device_head;
    uint8_t  command;
} AtaRegistersLba48;

typedef struct
{
    uint8_t  status;
    uint8_t  error;
    uint16_t sector_count;
    uint16_t lba_low;
    uint16_t lba_mid;
    uint16_t lba_high;
    uint8_t  device_head;
} AtaErrorRegistersLba48;

typedef struct
{
    AaruPacketHeader  hdr;
    uint32_t          buf_len;
    AtaRegistersLba48 registers;
    uint8_t           protocol;
    uint8_t           transfer_register;
    uint8_t           transfer_blocks;
    uint8_t           spare;
    uint32_t          timeout;
} AaruPacketCmdAtaLba48;

typedef struct
{
    AaruPacketHeader       hdr;
    uint32_t               buf_len;
    AtaErrorRegistersLba48 registers;
    uint32_t               duration;
    uint32_t               sense;
    uint32_t               error_no;
} AaruPacketResAtaLba48;

typedef struct
{
    uint8_t  command;
    uint8_t  write;
    uint8_t  application;
    uint32_t flags;
    uint32_t argument;
    uint32_t block_size;
    uint32_t blocks;
    uint32_t buf_len;
    uint32_t timeout;
} AaruCmdSdhci;

typedef struct
{
    AaruPacketHeader hdr;
    AaruCmdSdhci     command;
} AaruPacketCmdSdhci;

typedef struct
{
    uint32_t buf_len;
    uint32_t response[4];
    uint32_t duration;
    uint32_t sense;
    uint32_t error_no;
} AaruResSdhci;

typedef struct
{
    AaruPacketHeader hdr;
    AaruResSdhci     res;
} AaruPacketResSdhci;

typedef struct
{
    AaruPacketHeader hdr;
} AaruPacketCmdGetDeviceType;

typedef struct
{
    AaruPacketHeader hdr;
    int32_t          device_type;
} AaruPacketResGetDeviceType;

typedef struct
{
    AaruPacketHeader hdr;
} AaruPacketCmdGetSdhciRegisters;

typedef struct
{
    AaruPacketHeader hdr;
    uint8_t          is_sdhci;
    char             csd[16];
    char             cid[16];
    char             ocr[4];
    char             scr[8];
    uint32_t         csd_len;
    uint32_t         cid_len;
    uint32_t         ocr_len;
    uint32_t         scr_len;
} AaruPacketResGetSdhciRegisters;

typedef struct
{
    AaruPacketHeader hdr;
} AaruPacketCmdGetUsbData;

typedef struct
{
    AaruPacketHeader hdr;
    uint8_t          is_usb;
    uint16_t         desc_len;
    char             descriptors[65536];
    uint16_t         id_vendor;
    uint16_t         id_product;
    char             manufacturer[256];
    char             product[256];
    char             serial[256];
} AaruPacketResGetUsbData;

typedef struct
{
    AaruPacketHeader hdr;
} AaruPacketCmdGetFireWireData;

typedef struct
{
    AaruPacketHeader hdr;
    uint8_t          is_firewire;
    uint32_t         id_model;
    uint32_t         id_vendor;
    uint64_t         guid;
    char             vendor[256];
    char             model[256];
} AaruPacketResGetFireWireData;

typedef struct
{
    AaruPacketHeader hdr;
} AaruPacketCmdGetPcmciaData;

typedef struct
{
    AaruPacketHeader hdr;
    uint8_t          is_pcmcia;
    uint16_t         cis_len;
    char             cis[65536];
} AaruPacketResGetPcmciaData;

typedef struct
{
    AaruPacketHeader hdr;
} AaruPacketCmdClose;

typedef struct
{
    AaruPacketHeader hdr;
} AaruPacketCmdAmIRoot;

typedef struct
{
    AaruPacketHeader hdr;
    uint32_t         am_i_root;
} AaruPacketResAmIRoot;

typedef struct
{
    AaruPacketHeader hdr;
    uint64_t         cmd_count;
    AaruCmdSdhci     commands[0];
} AaruPacketMultiCmdSdhci;

typedef struct
{
    AaruPacketHeader hdr;
    uint64_t         cmd_count;
    AaruResSdhci     responses[0];
} AaruPacketMultiResSdhci;

typedef struct
{
    AaruPacketHeader hdr;
} AaruPacketCmdReOpen;

typedef struct
{
    AaruPacketHeader hdr;
    uint64_t         offset;
                     uint32_t length;
} AaruPacketCmdOsRead;

typedef struct
{
    AaruPacketHeader hdr;
    int32_t error_no;
    uint32_t duration;
} AaruPacketResOsRead;

#pragma pack(pop)

typedef struct
{
    uint32_t argument;
    uint32_t blocks;
    uint32_t block_size;
    uint32_t buf_len;
    char*    buffer;
    uint8_t  command;
    uint32_t flags;
    uint8_t  application;
    uint32_t response[4];
    uint8_t  write;
} MmcSingleCommand;

DeviceInfoList*  ListDevices();
void             FreeDeviceInfoList(DeviceInfoList* start);
uint16_t         DeviceInfoListCount(DeviceInfoList* start);
void*            DeviceOpen(const char* device_path);
void             DeviceClose(void* device_ctx);
int32_t          GetDeviceType(void* device_ctx);
int32_t          SendScsiCommand(void*     device_ctx,
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
int              Hexchr2Bin(const char hex, char* out);
size_t           Hexs2Bin(const char* hex, unsigned char** out);
int32_t          GetSdhciRegisters(void*     device_ctx,
                                   char**    csd,
                                   char**    cid,
                                   char**    ocr,
                                   char**    scr,
                                   uint32_t* csd_len,
                                   uint32_t* cid_len,
                                   uint32_t* ocr_len,
                                   uint32_t* scr_len);
uint8_t          GetUsbData(void*     device_ctx,
                            uint16_t* desc_len,
                            char*     descriptors,
                            uint16_t* id_vendor,
                            uint16_t* id_product,
                            char*     manufacturer,
                            char*     product,
                            char*     serial);
uint8_t          GetFireWireData(void*     device_ctx,
                                 uint32_t* id_model,
                                 uint32_t* id_vendor,
                                 uint64_t* guid,
                                 char*     vendor,
                                 char*     model);
uint8_t          GetPcmciaData(void* device_ctx, uint16_t* cis_len, char* cis);
int32_t          SendAtaChsCommand(void*                 device_ctx,
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
int32_t          SendAtaLba28Command(void*                   device_ctx,
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
int32_t          SendAtaLba48Command(void*                   device_ctx,
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
int32_t          SendSdhciCommand(void*     device_ctx,
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
                                  uint32_t* sense);
int32_t          SendMultiSdhciCommand(void*            device_ctx,
                                       uint64_t         count,
                                       MmcSingleCommand commands[],
                                       uint32_t*        duration,
                                       uint32_t*        sense);
int32_t          OsRead(void*            device_ctx,
                                       char *buffer,
                                       uint64_t         offset,
                                       uint32_t length,
                                       uint32_t*        duration);
AaruPacketHello* GetHello();
int              PrintNetworkAddresses();
char*            PrintIpv4Address(struct in_addr addr);
void*            NetSocket(uint32_t domain, uint32_t type, uint32_t protocol);
int32_t          NetBind(void* net_ctx, struct sockaddr* addr, socklen_t addrlen);
int32_t          NetListen(void* net_ctx, uint32_t backlog);
void*            NetAccept(void* net_ctx, struct sockaddr* addr, socklen_t* addrlen);
int32_t          NetRecv(void* net_ctx, void* buf, int32_t len, uint32_t flags);
int32_t          NetWrite(void* net_ctx, const void* buf, int32_t size);
int32_t          NetClose(void* net_ctx);
void             Initialize();
void             PlatformLoop(AaruPacketHello* pkt_server_hello);
void*            WorkingLoop(void* arguments);
uint8_t          AmIRoot();
int32_t          ReOpen(void *device_ctx, uint32_t* closeFailed);
#endif
