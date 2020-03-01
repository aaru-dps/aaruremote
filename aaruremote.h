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

#include <stddef.h>
#include <stdint.h>

#ifdef GEKKO
#include <network.h>
#ifndef MSG_PEEK
#define MSG_PEEK 0x02 // TODO: Untested, may not work
#endif
#elif _WIN32
#include <ws2tcpip.h>
#else
#include <netinet/in.h>
#include <sys/socket.h>
#endif

#define DICMOTE_NAME "DiscImageChef Remote Server"
#define DICMOTE_VERSION "0.99"
#define DICMOTE_PORT 6666
#define DICMOTE_REMOTE_ID 0x52434944 // "DICR"
#define DICMOTE_PACKET_ID 0x544B4350 // "PCKT"
#define DICMOTE_PACKET_VERSION 1
#define DICMOTE_PACKET_TYPE_NOP -1
#define DICMOTE_PACKET_TYPE_HELLO 1
#define DICMOTE_PACKET_TYPE_COMMAND_LIST_DEVICES 2
#define DICMOTE_PACKET_TYPE_RESPONSE_LIST_DEVICES 3
#define DICMOTE_PACKET_TYPE_COMMAND_OPEN_DEVICE 4
#define DICMOTE_PACKET_TYPE_COMMAND_SCSI 5
#define DICMOTE_PACKET_TYPE_RESPONSE_SCSI 6
#define DICMOTE_PACKET_TYPE_COMMAND_ATA_CHS 7
#define DICMOTE_PACKET_TYPE_RESPONSE_ATA_CHS 8
#define DICMOTE_PACKET_TYPE_COMMAND_ATA_LBA_28 9
#define DICMOTE_PACKET_TYPE_RESPONSE_ATA_LBA_28 10
#define DICMOTE_PACKET_TYPE_COMMAND_ATA_LBA_48 11
#define DICMOTE_PACKET_TYPE_RESPONSE_ATA_LBA_48 12
#define DICMOTE_PACKET_TYPE_COMMAND_SDHCI 13
#define DICMOTE_PACKET_TYPE_RESPONSE_SDHCI 14
#define DICMOTE_PACKET_TYPE_COMMAND_GET_DEVTYPE 15
#define DICMOTE_PACKET_TYPE_RESPONSE_GET_DEVTYPE 16
#define DICMOTE_PACKET_TYPE_COMMAND_GET_SDHCI_REGISTERS 17
#define DICMOTE_PACKET_TYPE_RESPONSE_GET_SDHCI_REGISTERS 18
#define DICMOTE_PACKET_TYPE_COMMAND_GET_USB_DATA 19
#define DICMOTE_PACKET_TYPE_RESPONSE_GET_USB_DATA 20
#define DICMOTE_PACKET_TYPE_COMMAND_GET_FIREWIRE_DATA 21
#define DICMOTE_PACKET_TYPE_RESPONSE_GET_FIREWIRE_DATA 22
#define DICMOTE_PACKET_TYPE_COMMAND_GET_PCMCIA_DATA 23
#define DICMOTE_PACKET_TYPE_RESPONSE_GET_PCMCIA_DATA 24
#define DICMOTE_PACKET_TYPE_COMMAND_CLOSE_DEVICE 25
#define DICMOTE_PACKET_TYPE_COMMAND_AM_I_ROOT 26
#define DICMOTE_PACKET_TYPE_RESPONSE_AM_I_ROOT 27
#define DICMOTE_PROTOCOL_MAX 1
#define DICMOTE_PACKET_NOP_REASON_OOO 0
#define DICMOTE_PACKET_NOP_REASON_NOT_IMPLEMENTED 1
#define DICMOTE_PACKET_NOP_REASON_NOT_RECOGNIZED 2
#define DICMOTE_PACKET_NOP_REASON_ERROR_LIST_DEVICES 3
#define DICMOTE_PACKET_NOP_REASON_OPEN_OK 4
#define DICMOTE_PACKET_NOP_REASON_OPEN_ERROR 5
#define DICMOTE_DEVICE_TYPE_UNKNOWN -1
#define DICMOTE_DEVICE_TYPE_ATA 1
#define DICMOTE_DEVICE_TYPE_ATAPI 2
#define DICMOTE_DEVICE_TYPE_SCSI 3
#define DICMOTE_DEVICE_TYPE_SECURE_DIGITAL 4
#define DICMOTE_DEVICE_TYPE_MMC 5
#define DICMOTE_DEVICE_TYPE_NVME 6
#define DICMOTE_SCSI_DIRECTION_UNSPECIFIED -1
#define DICMOTE_SCSI_DIRECTION_NONE 0
#define DICMOTE_SCSI_DIRECTION_OUT 1
#define DICMOTE_SCSI_DIRECTION_IN 2
#define DICMOTE_SCSI_DIRECTION_INOUT 3
#define DICMOTE_ATA_PROTOCOL_HARD_RESET 0
#define DICMOTE_ATA_PROTOCOL_SOFT_RESET 1
#define DICMOTE_ATA_PROTOCOL_NO_DATA 3
#define DICMOTE_ATA_PROTOCOL_PIO_IN 4
#define DICMOTE_ATA_PROTOCOL_PIO_OUT 5
#define DICMOTE_ATA_PROTOCOL_DMA 6
#define DICMOTE_ATA_PROTOCOL_DMA_QUEUED 7
#define DICMOTE_ATA_PROTOCOL_DEVICE_DIAGNOSTIC 8
#define DICMOTE_ATA_PROTOCOL_DEVICE_RESET 9
#define DICMOTE_ATA_PROTOCOL_UDMA_IN 10
#define DICMOTE_ATA_PROTOCOL_UDMA_OUT 11
#define DICMOTE_ATA_PROTOCOL_FPDMA 12
#define DICMOTE_ATA_PROTOCOL_RETURN_RESPONSE 15
#define DICMOTE_ATA_TRANSFER_REGISTER_NONE 0
#define DICMOTE_ATA_TRANSFER_REGISTER_FEATURE 1
#define DICMOTE_ATA_TRANSFER_REGISTER_SECTOR_COUNT 2
#define DICMOTE_ATA_TRANSFER_REGISTER_SPTSIU 3
#define DICMOTE_MMC_RESPONSE_PRESENT (1 << 0)
#define DICMOTE_MMC_RESPONSE_136 (1 << 1)
#define DICMOTE_MMC_RESPONSE_CRC (1 << 2)
#define DICMOTE_MMC_RESPONSE_BUSY (1 << 3)
#define DICMOTE_MMC_RESPONSE_OPCODE (1 << 4)
#define DICMOTE_MMC_COMMAND_MASK (3 << 5)
#define DICMOTE_MMC_COMMAND_AC (0 << 5)
#define DICMOTE_MMC_COMMAND_ADTC (1 << 5)
#define DICMOTE_MMC_COMMAND_BC (2 << 5)
#define DICMOTE_MMC_COMMAND_BCR (3 << 5)
#define DICMOTE_MMC_RESPONSE_SPI_S1 (1 << 7)
#define DICMOTE_MMC_RESPONSE_SPI_S2 (1 << 8)
#define DICMOTE_MMC_RESPONSE_SPI_B4 (1 << 9)
#define DICMOTE_MMC_RESPONSE_SPI_BUSY (1 << 10)
#define DICMOTE_MMC_RESPONSE_NONE (0)
#define DICMOTE_MMC_RESPONSE_R1 DICMOTE_MMC_RESPONSE_PRESENT | DICMOTE_MMC_RESPONSE_CRC | DICMOTE_MMC_RESPONSE_OPCODE
#define DICMOTE_MMC_RESPONSE_R1B                                                                                       \
    DICMOTE_MMC_RESPONSE_PRESENT | DICMOTE_MMC_RESPONSE_CRC | DICMOTE_MMC_RESPONSE_OPCODE | DICMOTE_MMC_RESPONSE_BUSY
#define DICMOTE_MMC_RESPONSE_R2 DICMOTE_MMC_RESPONSE_PRESENT | DICMOTE_MMC_RESPONSE_136 | DICMOTE_MMC_RESPONSE_CRC
#define DICMOTE_MMC_RESPONSE_R3 DICMOTE_MMC_RESPONSE_PRESENT
#define DICMOTE_MMC_RESPONSE_R4 DICMOTE_MMC_RESPONSE_PRESENT
#define DICMOTE_MMC_RESPONSE_R5 DICMOTE_MMC_RESPONSE_PRESENT | DICMOTE_MMC_RESPONSE_CRC | DICMOTE_MMC_RESPONSE_OPCODE
#define DICMOTE_MMC_RESPONSE_R6 DICMOTE_MMC_RESPONSE_PRESENT | DICMOTE_MMC_RESPONSE_CRC | DICMOTE_MMC_RESPONSE_OPCODE
#define DICMOTE_MMC_RESPONSE_R7 DICMOTE_MMC_RESPONSE_PRESENT | DICMOTE_MMC_RESPONSE_CRC | DICMOTE_MMC_RESPONSE_OPCODE
#define DICMOTE_MMC_RESPONSE_SPI_R1 DICMOTE_MMC_RESPONSE_SPI_S1
#define DICMOTE_MMC_RESPONSE_SPI_R1B DICMOTE_MMC_RESPONSE_SPI_S1 | DICMOTE_MMC_RESPONSE_SPI_BUSY
#define DICMOTE_MMC_RESPONSE_SPI_R2 DICMOTE_MMC_RESPONSE_SPI_S1 | DICMOTE_MMC_RESPONSE_SPI_S2
#define DICMOTE_MMC_RESPONSE_SPI_R3 DICMOTE_MMC_RESPONSE_SPI_S1 | DICMOTE_MMC_RESPONSE_SPI_B4
#define DICMOTE_MMC_RESPONSE_SPI_R4 DICMOTE_MMC_RESPONSE_SPI_S1 | DICMOTE_MMC_RESPONSE_SPI_B4
#define DICMOTE_MMC_RESPONSE_SPI_R5 DICMOTE_MMC_RESPONSE_SPI_S1 | DICMOTE_MMC_RESPONSE_SPI_S2
#define DICMOTE_MMC_RESPONSE_SPI_R7 DICMOTE_MMC_RESPONSE_SPI_S1 | DICMOTE_MMC_RESPONSE_SPI_B4

#pragma pack(push, 1)

typedef struct
{
    uint32_t remote_id;
    uint32_t packet_id;
    uint32_t len;
    uint8_t  version;
    int8_t   packet_type;
    char     spare[2];
} DicPacketHeader;

typedef struct
{
    DicPacketHeader hdr;
    char            application[128];
    char            version[64];
    uint8_t         max_protocol;
    char            spare[3];
    char            sysname[256];
    char            release[256];
    char            machine[256];
} DicPacketHello;

typedef struct
{
    DicPacketHeader hdr;
} DicPacketCmdListDevs;

typedef struct
{
    DicPacketHeader hdr;
    uint16_t        devices;
} DicPacketResListDevs;

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
    DicPacketHeader hdr;
    uint8_t         reason_code;
    char            spare[3];
    char            reason[256];
    int32_t         error_no;
} DicPacketNop;

typedef struct
{
    DicPacketHeader hdr;
    char            device_path[1024];
} DicPacketCmdOpen;

typedef struct
{
    DicPacketHeader hdr;
    uint32_t        cdb_len;
    uint32_t        buf_len;
    int32_t         direction;
    uint32_t        timeout;
} DicPacketCmdScsi;

typedef struct
{
    DicPacketHeader hdr;
    uint32_t        sense_len;
    uint32_t        buf_len;
    uint32_t        duration;
    uint32_t        sense;
    uint32_t        error_no;
} DicPacketResScsi;

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
    DicPacketHeader hdr;
    uint32_t        buf_len;
    AtaRegistersChs registers;
    uint8_t         protocol;
    uint8_t         transfer_register;
    uint8_t         transfer_blocks;
    uint8_t         spare;
    uint32_t        timeout;
} DicPacketCmdAtaChs;

typedef struct
{
    DicPacketHeader      hdr;
    uint32_t             buf_len;
    AtaErrorRegistersChs registers;
    uint32_t             duration;
    uint32_t             sense;
    uint32_t             error_no;
} DicPacketResAtaChs;

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
    DicPacketHeader   hdr;
    uint32_t          buf_len;
    AtaRegistersLba28 registers;
    uint8_t           protocol;
    uint8_t           transfer_register;
    uint8_t           transfer_blocks;
    uint8_t           spare;
    uint32_t          timeout;
} DicPacketCmdAtaLba28;

typedef struct
{
    DicPacketHeader        hdr;
    uint32_t               buf_len;
    AtaErrorRegistersLba28 registers;
    uint32_t               duration;
    uint32_t               sense;
    uint32_t               error_no;
} DicPacketResAtaLba28;

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
    DicPacketHeader   hdr;
    uint32_t          buf_len;
    AtaRegistersLba48 registers;
    uint8_t           protocol;
    uint8_t           transfer_register;
    uint8_t           transfer_blocks;
    uint8_t           spare;
    uint32_t          timeout;
} DicPacketCmdAtaLba48;

typedef struct
{
    DicPacketHeader        hdr;
    uint32_t               buf_len;
    AtaErrorRegistersLba48 registers;
    uint32_t               duration;
    uint32_t               sense;
    uint32_t               error_no;
} DicPacketResAtaLba48;

typedef struct
{
    DicPacketHeader hdr;
    uint8_t         command;
    uint8_t         write;
    uint8_t         application;
    uint32_t        flags;
    uint32_t        argument;
    uint32_t        block_size;
    uint32_t        blocks;
    uint32_t        buf_len;
    uint32_t        timeout;
} DicPacketCmdSdhci;

typedef struct
{
    DicPacketHeader hdr;
    uint32_t        buf_len;
    uint32_t        response[4];
    uint32_t        duration;
    uint32_t        sense;
    uint32_t        error_no;
} DicPacketResSdhci;

typedef struct
{
    DicPacketHeader hdr;
} DicPacketCmdGetDeviceType;

typedef struct
{
    DicPacketHeader hdr;
    int32_t         device_type;
} DicPacketResGetDeviceType;

typedef struct
{
    DicPacketHeader hdr;
} DicPacketCmdGetSdhciRegisters;

typedef struct
{
    DicPacketHeader hdr;
    uint8_t         is_sdhci;
    char            csd[16];
    char            cid[16];
    char            ocr[4];
    char            scr[8];
    uint32_t        csd_len;
    uint32_t        cid_len;
    uint32_t        ocr_len;
    uint32_t        scr_len;
} DicPacketResGetSdhciRegisters;

typedef struct
{
    DicPacketHeader hdr;
} DicPacketCmdGetUsbData;

typedef struct
{
    DicPacketHeader hdr;
    uint8_t         is_usb;
    uint16_t        desc_len;
    char            descriptors[65536];
    uint16_t        id_vendor;
    uint16_t        id_product;
    char            manufacturer[256];
    char            product[256];
    char            serial[256];
} DicPacketResGetUsbData;

typedef struct
{
    DicPacketHeader hdr;
} DicPacketCmdGetFireWireData;

typedef struct
{
    DicPacketHeader hdr;
    uint8_t         is_firewire;
    uint32_t        id_model;
    uint32_t        id_vendor;
    uint64_t        guid;
    char            vendor[256];
    char            model[256];
} DicPacketResGetFireWireData;

typedef struct
{
    DicPacketHeader hdr;
} DicPacketCmdGetPcmciaData;

typedef struct
{
    DicPacketHeader hdr;
    uint8_t         is_pcmcia;
    uint16_t        cis_len;
    char            cis[65536];
} DicPacketResGetPcmciaData;

typedef struct
{
    DicPacketHeader hdr;
} DicPacketCmdClose;

typedef struct
{
    DicPacketHeader hdr;
} DicPacketCmdAmIRoot;

typedef struct
{
    DicPacketHeader hdr;
    uint32_t        am_i_root;
} DicPacketResAmIRoot;

#pragma pack(pop)

DeviceInfoList* ListDevices();
void            FreeDeviceInfoList(DeviceInfoList* start);
uint16_t        DeviceInfoListCount(DeviceInfoList* start);
void*           DeviceOpen(const char* device_path);
void            DeviceClose(void* device_ctx);
int32_t         GetDeviceType(void* device_ctx);
int32_t         SendScsiCommand(void*     device_ctx,
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
int             Hexchr2Bin(const char hex, char* out);
size_t          Hexs2Bin(const char* hex, unsigned char** out);
int32_t         GetSdhciRegisters(void*     device_ctx,
                                  char**    csd,
                                  char**    cid,
                                  char**    ocr,
                                  char**    scr,
                                  uint32_t* csd_len,
                                  uint32_t* cid_len,
                                  uint32_t* ocr_len,
                                  uint32_t* scr_len);
uint8_t         GetUsbData(void*     device_ctx,
                           uint16_t* desc_len,
                           char*     descriptors,
                           uint16_t* id_vendor,
                           uint16_t* id_product,
                           char*     manufacturer,
                           char*     product,
                           char*     serial);
uint8_t         GetFireWireData(void*     device_ctx,
                                uint32_t* id_model,
                                uint32_t* id_vendor,
                                uint64_t* guid,
                                char*     vendor,
                                char*     model);
uint8_t         GetPcmciaData(void* device_ctx, uint16_t* cis_len, char* cis);
int32_t         SendAtaChsCommand(void*                 device_ctx,
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
int32_t         SendAtaLba28Command(void*                   device_ctx,
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
int32_t         SendAtaLba48Command(void*                   device_ctx,
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
int32_t         SendSdhciCommand(void*     device_ctx,
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
DicPacketHello* GetHello();
int             PrintNetworkAddresses();
char*           PrintIpv4Address(struct in_addr addr);
void*           NetSocket(uint32_t domain, uint32_t type, uint32_t protocol);
int32_t         NetBind(void* net_ctx, struct sockaddr* addr, socklen_t addrlen);
int32_t         NetListen(void* net_ctx, uint32_t backlog);
void*           NetAccept(void* net_ctx, struct sockaddr* addr, socklen_t* addrlen);
int32_t         NetRecv(void* net_ctx, void* buf, int32_t len, uint32_t flags);
int32_t         NetWrite(void* net_ctx, const void* buf, int32_t size);
int32_t         NetClose(void* net_ctx);
void            Initialize();
void            PlatformLoop(DicPacketHello* pkt_server_hello);
void*           WorkingLoop(void* arguments);
uint8_t         AmIRoot();
#endif
