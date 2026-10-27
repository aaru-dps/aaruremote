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

#ifndef AARUREMOTE_FREEBSD_FREEBSD_H_
#define AARUREMOTE_FREEBSD_FREEBSD_H_

// Included here as it seems to need to be after all others
#ifndef _STDIO_H_
#include <stdio.h>
#endif

#include <sys/param.h>

#include <camlib.h>
#include <time.h>

// CAM can send ATA commands since FreeBSD 8.0
#ifdef CAM_ATAIO_NEEDRESULT
#define HAS_CAM_ATA
#endif

// Old releases, such as FreeBSD 5.5, have no precise clocks, so use the plain realtime clock there
#ifndef CLOCK_REALTIME_PRECISE
#define CLOCK_REALTIME_PRECISE CLOCK_REALTIME
#endif

// Bidirectional CAM data transfers, called CAM_DIR_BOTH on current releases and CAM_DIR_RESV on old ones. It is an
// enumeration value, so it cannot be tested for with the preprocessor.
#define AARUREMOTE_CAM_DIR_BOTH 0x00000000

typedef struct
{
    char               device_path[4096];
    struct cam_device *device;
} DeviceContext;

#endif  // AARUREMOTE_FREEBSD_FREEBSD_H_
