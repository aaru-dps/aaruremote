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
#include <sys/ata.h>

#include <camlib.h>
#include <time.h>

// CAM can send ATA commands since FreeBSD 8.0
#ifdef CAM_ATAIO_NEEDRESULT
#define HAS_CAM_ATA
#endif

// Before FreeBSD 6 the ata(4) control device, /dev/ata, takes raw ATA commands for the disks it attaches
#if defined(IOCATA) && defined(ATAREQUEST)
#define HAS_ATA_IOCTL
#endif

// From FreeBSD 6 until ata(4) was replaced by CAM, the disks attached to ata(4) take raw ATA commands themselves
#if !defined(HAS_ATA_IOCTL) && defined(IOCATAREQUEST) && defined(IOCATAGPARM)
#define HAS_ATA_DEVICE_IOCTL
#endif

// Clients send timeouts in milliseconds, but ata(4) takes seconds, and times a request out immediately when given 0,
// so then use the timeout it uses for its own requests
#define AARUREMOTE_ATA_IOCTL_TIMEOUT(ms) ((ms) ? ((ms) + 999) / 1000 : 5)

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
    int                ata_fd;      // ata(4) device taking raw ATA commands, for ATA disks not attached to CAM
    int                ata_channel; // ata(4) channel of the disk
    int                ata_unit;    // ata(4) unit of the disk, 0 for master and 1 for slave
    int                fd;          // The disk itself, for operating system reads of ata(4) disks
} DeviceContext;

#endif  // AARUREMOTE_FREEBSD_FREEBSD_H_
