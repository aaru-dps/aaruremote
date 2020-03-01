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

#include "win32.h"

#include <stdint.h>

uint8_t GetFireWireData(void*     device_ctx,
                        uint32_t* id_model,
                        uint32_t* id_vendor,
                        uint64_t* guid,
                        char*     vendor,
                        char*     model)
{
    DeviceContext* ctx = device_ctx;

    if(!ctx) return 0;

    // TODO: Implement
    return 0;
}
