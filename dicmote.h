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
#define DICMOTE_PACKET_ID "DICPACKT"
#define DICMOTE_PACKET_VERSION 1
#define DICMOTE_PACKET_TYPE_HELLO 1
#define DICMOTE_PROTOCOL_MAX 1

#pragma pack(push, 1)

typedef struct
{
    char     id[8];
    uint32_t len;
    uint8_t  version;
    uint8_t  packet_type;
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

#pragma pack(pop)

#endif
