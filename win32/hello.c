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

#include "../dicmote.h"
#include "../endian.h"

#include <malloc.h>
#include <string.h>

DicPacketHello* GetHello()
{
    DicPacketHello* pkt_server_hello;

    pkt_server_hello = malloc(sizeof(DicPacketHello));

    if(!pkt_server_hello) { return 0; }

    memset(pkt_server_hello, 0, sizeof(DicPacketHello));

    pkt_server_hello->hdr.remote_id   = htole32(DICMOTE_REMOTE_ID);
    pkt_server_hello->hdr.packet_id   = htole32(DICMOTE_PACKET_ID);
    pkt_server_hello->hdr.len         = htole32(sizeof(DicPacketHello));
    pkt_server_hello->hdr.version     = DICMOTE_PACKET_VERSION;
    pkt_server_hello->hdr.packet_type = DICMOTE_PACKET_TYPE_HELLO;
    strncpy(pkt_server_hello->application, DICMOTE_NAME, sizeof(DICMOTE_NAME));
    strncpy(pkt_server_hello->version, DICMOTE_VERSION, sizeof(DICMOTE_VERSION));
    pkt_server_hello->max_protocol = DICMOTE_PROTOCOL_MAX;
    strncpy(pkt_server_hello->sysname, "Windows", 255);
    strncpy(pkt_server_hello->release, "TODO", 255);
    strncpy(pkt_server_hello->machine, "TODO", 255);

    return pkt_server_hello;
}
