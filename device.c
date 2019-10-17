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

#if defined(__linux__) && !defined(__ANDROID__)
#include "linux/linux.h"
#endif

int DeviceOpen(const char* devicePath)
{
#if defined(__linux__) && !defined(__ANDROID__)
    return linux_open_device(devicePath);
#else
    return -1;
#endif
}

int32_t GetDeviceType(const char* devicePath)
{
#if defined(__linux__) && !defined(__ANDROID__)
    return linux_get_device_type(devicePath);
#else
    return DICMOTE_DEVICE_TYPE_UNKNOWN;
#endif
}

int32_t GetSdhciRegisters(const char* devicePath,
                          char**      csd,
                          char**      cid,
                          char**      ocr,
                          char**      scr,
                          uint32_t*   csd_len,
                          uint32_t*   cid_len,
                          uint32_t*   ocr_len,
                          uint32_t*   scr_len)
{
#if defined(__linux__) && !defined(__ANDROID__)
    return linux_get_sdhci_registers(devicePath, csd, cid, ocr, scr, csd_len, cid_len, ocr_len, scr_len);
#else
    return 0;
#endif
}