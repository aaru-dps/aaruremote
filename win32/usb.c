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

#include "win32.h"

#include <stdint.h>

uint8_t Win32GetUsbData(void*     device_ctx,
                        uint16_t* desc_len,
                        char*     descriptors,
                        uint16_t* id_vendor,
                        uint16_t* id_product,
                        char*     manufacturer,
                        char*     product,
                        char*     serial)
{
    Win32DeviceContext* ctx = device_ctx;

    if(!ctx) return -1;

    // TODO: Implement
    return -1;
}
