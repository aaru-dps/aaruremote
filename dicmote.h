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

#pragma pack(pop)

DeviceInfoList* ListDevices();
void            FreeDeviceInfoList(DeviceInfoList* start);
uint16_t        DeviceInfoListCount(DeviceInfoList* start);
int             DeviceOpen(const char* devicePath);

#endif
