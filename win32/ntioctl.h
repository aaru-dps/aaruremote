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

#include <windows.h>

#ifdef HAS_NTDDSCSI_H
#include <ntddscsi.h>
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

#endif // DICREMOTE_WIN32_NTIOCTL_H_
