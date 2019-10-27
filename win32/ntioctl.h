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

#ifndef DICREMOTE_WIN32_NTIOCTL_H_
#define DICREMOTE_WIN32_NTIOCTL_H_

#include <winsock2.h>
#include <windows.h>

#ifdef HAS_NTDDSCSI_H
#include <ntddscsi.h>
#endif

#ifdef HAVE_SFFDISK_H
#include <sffdisk.h>
#endif

#ifdef HAVE_SDDEF_H
#include <sddef.h>
#endif

#ifndef ATA_FLAGS_DRDY_REQUIRED
#define ATA_FLAGS_DRDY_REQUIRED (1 << 0)
#endif

#ifndef ATA_FLAGS_DATA_IN
#define ATA_FLAGS_DATA_IN (1 << 1)
#endif

#ifndef ATA_FLAGS_DATA_OUT
#define ATA_FLAGS_DATA_OUT (1 << 2)
#endif

#ifndef ATA_FLAGS_48BIT_COMMAND
#define ATA_FLAGS_48BIT_COMMAND (1 << 3)
#endif

#ifndef ATA_FLAGS_USE_DMA
#define ATA_FLAGS_USE_DMA (1 << 4)
#endif

#ifndef IOCTL_ATA_PASS_THROUGH
#define IOCTL_ATA_PASS_THROUGH 0x4D02C
#endif

#ifndef SCSI_IOCTL_DATA_OUT
#define SCSI_IOCTL_DATA_OUT 0
#endif

#ifndef SCSI_IOCTL_DATA_IN
#define SCSI_IOCTL_DATA_IN 1
#endif

#ifndef SCSI_IOCTL_DATA_UNSPECIFIED
#define SCSI_IOCTL_DATA_UNSPECIFIED 2
#endif

#ifndef SCSI_IOCTL_DATA_BIDIRECTIONAL
#define SCSI_IOCTL_DATA_BIDIRECTIONAL 3
#endif

#ifndef IOCTL_SCSI_PASS_THROUGH_DIRECT
#define IOCTL_SCSI_PASS_THROUGH_DIRECT 0x4D014
#endif

#ifndef IOCTL_SFFDISK_DEVICE_COMMAND
#define IOCTL_SFFDISK_DEVICE_COMMAND 0x79E84
#endif

#ifndef IOCTL_SFFDISK_QUERY_DEVICE_PROTOCOL
#define IOCTL_SFFDISK_QUERY_DEVICE_PROTOCOL 0x71E80
#endif

#ifndef GUID_SFF_PROTOCOL_SD
#define GUID_SFF_PROTOCOL_SD                                                                                           \
    {                                                                                                                  \
        0xAD7536A8, 0xD055, 0x4c40, { 0xAA, 0x4D, 0x96, 0x31, 0x2D, 0xDB, 0x6B, 0x38 }                                 \
    }
#endif

#ifndef GUID_SFF_PROTOCOL_MMC
#define GUID_SFF_PROTOCOL_MMC                                                                                          \
    {                                                                                                                  \
        0x77274D3F, 0x2365, 0x4491, { 0xA0, 0x30, 0x8B, 0xB4, 0x4A, 0xE6, 0x00, 0x97 }                                 \
    }
#endif

#ifndef HAS_SPTD
typedef struct _SCSI_PASS_THROUGH_DIRECT
{
    USHORT Length;
    UCHAR  ScsiStatus;
    UCHAR  PathId;
    UCHAR  TargetId;
    UCHAR  Lun;
    UCHAR  CdbLength;
    UCHAR  SenseInfoLength;
    UCHAR  DataIn;
    ULONG  DataTransferLength;
    ULONG  TimeOutValue;
    PVOID  DataBuffer;
    ULONG  SenseInfoOffset;
    UCHAR  Cdb[16];
} SCSI_PASS_THROUGH_DIRECT, *PSCSI_PASS_THROUGH_DIRECT;
#endif

#ifndef HAS_APTE
typedef struct _ATA_PASS_THROUGH_EX
{
    USHORT    Length;
    USHORT    AtaFlags;
    UCHAR     PathId;
    UCHAR     TargetId;
    UCHAR     Lun;
    UCHAR     ReservedAsUchar;
    ULONG     DataTransferLength;
    ULONG     TimeOutValue;
    ULONG     ReservedAsUlong;
    ULONG_PTR DataBufferOffset;
    UCHAR     PreviousTaskFile[8];
    UCHAR     CurrentTaskFile[8];
} ATA_PASS_THROUGH_EX, *PATA_PASS_THROUGH_EX;
#endif

#ifndef HAS_SDCD
typedef enum
{
    SFFDISK_DC_GET_VERSION,
    SFFDISK_DC_LOCK_CHANNEL,
    SFFDISK_DC_UNLOCK_CHANNEL,
    SFFDISK_DC_DEVICE_COMMAND
} SFFDISK_DCMD;

typedef struct _SFFDISK_DEVICE_COMMAND_DATA
{
    USHORT       HeaderSize;
    USHORT       Reserved;
    SFFDISK_DCMD Command;
    USHORT       ProtocolArgumentSize;
    ULONG        DeviceDataBufferSize;
    ULONG_PTR    Information;
    UCHAR        Data[];
} SFFDISK_DEVICE_COMMAND_DATA, *PSFFDISK_DEVICE_COMMAND_DATA;

typedef struct _SFFDISK_QUERY_DEVICE_PROTOCOL_DATA
{
    USHORT Size;
    USHORT Reserved;
    GUID   ProtocolGUID;
} SFFDISK_QUERY_DEVICE_PROTOCOL_DATA, *PSFFDISK_QUERY_DEVICE_PROTOCOL_DATA;
#endif

#ifndef SDCMDD
typedef UCHAR SD_COMMAND_CODE;

typedef enum
{
    SDCC_STANDARD,
    SDCC_APP_CMD
} SD_COMMAND_CLASS;

typedef enum
{
    SDTD_UNSPECIFIED = 0,
    SDTD_READ        = 1,
    SDTD_WRITE       = 2
} SD_TRANSFER_DIRECTION;

typedef enum
{
    SDTT_UNSPECIFIED,
    SDTT_CMD_ONLY,
    SDTT_SINGLE_BLOCK,
    SDTT_MULTI_BLOCK,
    SDTT_MULTI_BLOCK_NO_CMD12
} SD_TRANSFER_TYPE;

typedef enum
{
    SDRT_UNSPECIFIED,
    SDRT_NONE,
    SDRT_1,
    SDRT_1B,
    SDRT_2,
    SDRT_3,
    SDRT_4,
    SDRT_5,
    SDRT_5B,
    SDRT_6
} SD_RESPONSE_TYPE;

typedef struct _SDCMD_DESCRIPTOR
{
    SD_COMMAND_CODE       Cmd;
    SD_COMMAND_CLASS      CmdClass;
    SD_TRANSFER_DIRECTION TransferDirection;
    SD_TRANSFER_TYPE      TransferType;
    SD_RESPONSE_TYPE      ResponseType;
} SDCMD_DESCRIPTOR, *PSDCMD_DESCRIPTOR;
#endif

#endif // DICREMOTE_WIN32_NTIOCTL_H_
