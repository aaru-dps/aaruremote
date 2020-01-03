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

#ifndef DICREMOTE_WII_WII_H_
#define DICREMOTE_WII_WII_H_

#include "../dicmote.h"

#define DICREMOTE_WII_DEVICE_FD_NAND 1
#define DICREMOTE_WII_DEVICE_FD_DVD 2
#define DICREMOTE_WII_DEVICE_FD_SD 3
#define DICREMOTE_WII_DEVICE_FD_GCMM_A 4
#define DICREMOTE_WII_DEVICE_FD_GCMM_B 5

#define DICREMOTE_WII_IOCTL_SD_GET_DEVICE_STATUS 0x0B
#define DICREMOTE_WII_SD_INSERTED 1

#define DICREMOTE_WII_DEVICE_PATH_NAND "/dev/flash"
#define DICREMOTE_WII_DEVICE_PATH_DVD "/dev/di"
#define DICREMOTE_WII_DEVICE_PATH_SD "/dev/sdio/slot0"
#define DICREMOTE_WII_DEVICE_PATH_GCMM_A "/dev/gcmm/slota"
#define DICREMOTE_WII_DEVICE_PATH_GCMM_B "/dev/gcmm/slotb"

typedef struct
{
    s32 ios_fd;
    s32 dev_type;
} WiiDeviceContext;

typedef struct
{
    s32 fd;
} WiiNetworkContext;

DeviceInfoList* WiiListDevices();

#endif // DICREMOTE_WII_WII_H_
