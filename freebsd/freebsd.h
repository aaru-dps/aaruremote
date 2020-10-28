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

#ifndef AARUREMOTE_FREEBSD_FREEBSD_H_
#define AARUREMOTE_FREEBSD_FREEBSD_H_

// Included here as it seems to need to be after all others
#include <stdio.h>
#include <camlib.h>

typedef struct
{
    int  fd;
    char device_path[4096];
    struct cam_device *device;
} DeviceContext;

#endif // AARUREMOTE_FREEBSD_FREEBSD_H_
