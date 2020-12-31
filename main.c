/*
 * This file is part of the Aaru Remote Server.
 * Copyright (c) 2019-2021 Natalia Portillo.
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

#include <errno.h>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>

#include "win32/win32.h"
#else
#include <stdint.h>
#endif

#include "aaruremote.h"

int main()
{
    AaruPacketHello* pkt_server_hello;
    int              ret;

    Initialize();

    printf("Aaru Remote Server %s\n", AARUREMOTE_VERSION);
    printf("Copyright (C) 2019-2021 Natalia Portillo\n");

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