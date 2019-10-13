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

#ifndef DICMOTE_H
#define DICMOTE_H

#include <stdint.h>
#define DICMOTE_NAME "DiscImageChef Remote Server"
#define DICMOTE_VERSION "0.99"
#define DICMOTE_PORT 6666
#define DICMOTE_PACKET_ID 0x6873678065677584 // "DICPACKT"
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
#define DICMOTE_PACKET_TYPE_COMMAND_ATA_LBA28 9
#define DICMOTE_PACKET_TYPE_RESPONSE_ATA_LBA28 10
#define DICMOTE_PACKET_TYPE_COMMAND_ATA_LBA48 11
#define DICMOTE_PACKET_TYPE_RESPONSE_ATA_LBA48 12
#define DICMOTE_PACKET_TYPE_COMMAND_SDHCI 13
#define DICMOTE_PACKET_TYPE_RESPONSE_SDHCI 14
#define DICMOTE_PROTOCOL_MAX 1
#define DICMOTE_PACKET_NOP_REASON_OOO 0
#define DICMOTE_PACKET_NOP_REASON_NOT_IMPLEMENTED 1
#define DICMOTE_PACKET_NOP_REASON_NOT_RECOGNIZED 2
#define DICMOTE_PACKET_NOP_REASON_ERROR_LIST_DEVICES 3
#define DICMOTE_PACKET_NOP_REASON_OPEN_OK 4
#define DICMOTE_PACKET_NOP_REASON_OPEN_ERROR 5

#pragma pack(push, 1)

typedef struct
{
    uint64_t id;
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
    int32_t         errorNo;
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
    uint32_t        errno;
} DicPacketResScsi;

typedef struct
{
    uint8_t Feature;
    uint8_t SectorCount;
    uint8_t Sector;
    uint8_t CylinderLow;
    uint8_t CylinderHigh;
    uint8_t DeviceHead;
    uint8_t Command;
} AtaRegistersChs;

typedef struct
{
    uint8_t Status;
    uint8_t Error;
    uint8_t SectorCount;
    uint8_t Sector;
    uint8_t CylinderLow;
    uint8_t CylinderHigh;
    uint8_t DeviceHead;
} AtaErrorRegistersChs;

typedef struct
{
    DicPacketHeader hdr;
    uint32_t        buf_len;
    AtaRegistersChs registers;
    uint8_t         protocol;
    uint8_t         transferRegister;
    uint8_t         transferBlocks;
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
    uint32_t             errno;
} DicPacketResAtaChs;

typedef struct
{
    uint8_t Feature;
    uint8_t SectorCount;
    uint8_t LbaLow;
    uint8_t LbaMid;
    uint8_t LbaHigh;
    uint8_t DeviceHead;
    uint8_t Command;
} AtaRegistersLba28;

typedef struct
{
    uint8_t Status;
    uint8_t Error;
    uint8_t SectorCount;
    uint8_t LbaLow;
    uint8_t LbaMid;
    uint8_t LbaHigh;
    uint8_t DeviceHead;
} AtaErrorRegistersLba28;

typedef struct
{
    DicPacketHeader   hdr;
    uint32_t          buf_len;
    AtaRegistersLba28 registers;
    uint8_t           protocol;
    uint8_t           transferRegister;
    uint8_t           transferBlocks;
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
    uint32_t               errno;
} DicPacketResAtaLba28;

typedef struct
{
    uint16_t Feature;
    uint16_t SectorCount;
    uint16_t LbaLow;
    uint16_t LbaMid;
    uint16_t LbaHigh;
    uint8_t  DeviceHead;
    uint8_t  Command;
} AtaRegistersLba48;

typedef struct
{
    uint8_t  Status;
    uint8_t  Error;
    uint16_t SectorCount;
    uint16_t LbaLow;
    uint16_t LbaMid;
    uint16_t LbaHigh;
    uint8_t  DeviceHead;
} AtaErrorRegistersLba48;

typedef struct
{
    DicPacketHeader   hdr;
    uint32_t          buf_len;
    AtaRegistersLba48 registers;
    uint8_t           protocol;
    uint8_t           transferRegister;
    uint8_t           transferBlocks;
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
    uint32_t               errno;
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
    uint32_t        errno;
} DicPacketResSdhci;
#pragma pack(pop)

DeviceInfoList* ListDevices();
void            FreeDeviceInfoList(DeviceInfoList* start);
uint16_t        DeviceInfoListCount(DeviceInfoList* start);
int             DeviceOpen(const char* devicePath);

#endif
