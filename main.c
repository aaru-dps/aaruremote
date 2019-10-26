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

#include "dicmote.h"

#include <stdio.h>
#include <stdlib.h>

int main()
{
    DicPacketHello*                pkt_server_hello;
    int                            ret;

    Initialize();

    printf("DiscImageChef Remote Server %s\n", DICMOTE_VERSION);
    printf("Copyright (C) 2019 Natalia Portillo\n");

    pkt_server_hello = GetHello();

    if(!pkt_server_hello)
    {
        printf("Error %d getting system version.\n", errno);
        return 1;
    }

    printf(
        "Running under %s %s (%s).\n", pkt_server_hello->sysname, pkt_server_hello->release, pkt_server_hello->machine);

    ret = PrintNetworkAddresses();

    if(ret)
    {
        printf("Error %d enumerating interfaces\n", errno);
        return 1;
    }

    PlatformLoop(pkt_server_hello);
}