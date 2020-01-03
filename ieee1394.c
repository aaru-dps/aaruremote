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

#include <stdint.h>

#if defined(__linux__) && !defined(__ANDROID__)
#include "linux/linux.h"
#elif defined(WIN32)
#include "win32/win32.h"
#endif

uint8_t GetFireWireData(void*     device_ctx,
                        uint32_t* id_model,
                        uint32_t* id_vendor,
                        uint64_t* guid,
                        char*     vendor,
                        char*     model)
{
#if defined(__linux__) && !defined(__ANDROID__)
    return LinuxGetIeee1394Data(device_ctx, id_model, id_vendor, guid, vendor, model);
#elif defined(WIN32)
    return Win32GetIeee1394Data(device_ctx, id_model, id_vendor, guid, vendor, model);
#else
    return 0;
#endif
}