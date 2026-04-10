/*
 * This file is part of the Aaru Remote Server.
 * Copyright (c) 2019-2026 Natalia Portillo.
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

#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>

#include "win32/win32.h"
#else
#include <stdint.h>
#endif

#include "aaruremote.h"

void FreeDeviceInfoList(DeviceInfoList *start)
{
    DeviceInfoList *current;

    while(start)
    {
        current = start;
        start   = current->next;
        free(current);
    }
}

uint16_t DeviceInfoListCount(DeviceInfoList *start)
{
    uint16_t        count = 0;
    DeviceInfoList *current;

    while(start)
    {
        current = start;
        start   = current->next;
        count++;
    }

    return count;
}

void PrintDeviceList()
{
    DeviceInfoList *current;
    DeviceInfoList *start;
    uint16_t        count;
    uint16_t        i = 1;

    start = ListDevices();

    if(!start)
    {
        printf("Could not get device list.\n");
        return;
    }

    count = DeviceInfoListCount(start);
    printf("Detected %u device(s):\n", count);
    printf("+----+--------------------------+------------------+--------------------------+----------------------+-----"
           "---------+-----------+\n");
    printf("| #  | Path                     | Vendor           | Model                    | Serial               | Bus "
           "         | Supported |\n");
    printf("+----+--------------------------+------------------+--------------------------+----------------------+-----"
           "---------+-----------+\n");

    current = start;

    while(current)
    {
        printf("| %-2u | %-24.24s | %-16.16s | %-24.24s | %-20.20s | %-12.12s | %-9.9s |\n", i,
               current->this.path[0] ? current->this.path : "(unknown)",
               current->this.vendor[0] ? current->this.vendor : "(unknown)",
               current->this.model[0] ? current->this.model : "(unknown)",
               current->this.serial[0] ? current->this.serial : "(unknown)",
               current->this.bus[0] ? current->this.bus : "(unknown)", current->this.supported ? "yes" : "no");
        current = current->next;
        i++;
    }

    printf("+----+--------------------------+------------------+--------------------------+----------------------+-----"
           "---------+-----------+\n");

    FreeDeviceInfoList(start);
}