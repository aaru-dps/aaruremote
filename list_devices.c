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

#include "dicmote.h"

#if defined(__linux__) && !defined(__ANDROID__)
#include "linux/linux.h"
#elif defined(GEKKO)
#include "wii/wii.h"
#elif defined(WIN32)
#include "win32/win32.h"
#endif

#include <stdlib.h>

DeviceInfoList* ListDevices()
{
#if defined(__linux__) && !defined(__ANDROID__)
    return LinuxListDevices();
#elif defined(GEKKO)
    return WiiListDevices();
#elif defined(WIN32)
    return Win32ListDevices();
#else
    return NULL;
#endif
}

void FreeDeviceInfoList(DeviceInfoList* start)
{
    DeviceInfoList* current;

    while(start)
    {
        current = start;
        start   = current->next;
        free(current);
    }
}

uint16_t DeviceInfoListCount(DeviceInfoList* start)
{
    uint16_t        count = 0;
    DeviceInfoList* current;

    while(start)
    {
        current = start;
        start   = current->next;
        count++;
    }

    return count;
}