/*
 * This file is part of the DiscImageChef Remote Server.
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

#if defined(__linux__) && !defined(__ANDROID__)
#include "linux/linux.h"
#elif defined(WIN32)
#include "win32/win32.h"
#endif

#include "dicmote.h"

#include <stdint.h>

void* DeviceOpen(const char* device_path)
{
#if defined(__linux__) && !defined(__ANDROID__)
    return LinuxOpenDevice(device_path);
#elif defined(WIN32)
    return Win32OpenDevice(device_path);
#else
    return NULL;
#endif
}

void DeviceClose(void* device_ctx)
{
#if defined(__linux__) && !defined(__ANDROID__)
    return LinuxCloseDevice(device_ctx);
#elif defined(WIN32)
    return Win32CloseDevice(device_ctx);
#endif
}

int32_t GetDeviceType(void* device_ctx)
{
#if defined(__linux__) && !defined(__ANDROID__)
    return LinuxGetDeviceType(device_ctx);
#elif defined(WIN32)
    return Win32GetDeviceType(device_ctx);
#else
    return DICMOTE_DEVICE_TYPE_UNKNOWN;
#endif
}

int32_t GetSdhciRegisters(void*     device_ctx,
                          char**    csd,
                          char**    cid,
                          char**    ocr,
                          char**    scr,
                          uint32_t* csd_len,
                          uint32_t* cid_len,
                          uint32_t* ocr_len,
                          uint32_t* scr_len)
{
#if defined(__linux__) && !defined(__ANDROID__)
    return LinuxGetSdhciRegisters(device_ctx, csd, cid, ocr, scr, csd_len, cid_len, ocr_len, scr_len);
#elif defined(WIN32)
    return Win32GetSdhciRegisters(device_ctx, csd, cid, ocr, scr, csd_len, cid_len, ocr_len, scr_len);
#else
    return 0;
#endif
}