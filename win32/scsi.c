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

#include "../dicmote.h"
#include "win32.h"

#include <ntddscsi.h>
#include <stdint.h>

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

int32_t Win32SendScsiCommand(void*     device_ctx,
                             char*     cdb,
                             char*     buffer,
                             char**    sense_buffer,
                             uint32_t  timeout,
                             int32_t   direction,
                             uint32_t* duration,
                             uint32_t* sense,
                             uint32_t  cdb_len,
                             uint32_t* buf_len,
                             uint32_t* sense_len)
{
    Win32DeviceContext*       ctx = device_ctx;
    PSCSI_PASS_THROUGH_DIRECT sptd;
    PVOID                     sptd_and_sense;
    UCHAR                     dir;
    DWORD                     sptd_and_sense_len;
    DWORD                     k     = 0;
    DWORD                     error = 0;
    BOOL                      hasError;

    *duration          = 0;
    *sense_len         = 32;
    sptd_and_sense_len = *sense_len + sizeof(SCSI_PASS_THROUGH_DIRECT);

    if(!ctx) return -1;

    sptd_and_sense = malloc(sptd_and_sense_len);

    if(!*sense_buffer) return -1;

    memset(&sptd_and_sense, 0, sptd_and_sense_len);

    *sense_buffer = malloc(*sense_len);

    if(!*sense_buffer)
    {
        free(sptd_and_sense);
        return -1;
    }

    switch(direction)
    {
        case DICMOTE_SCSI_DIRECTION_IN: dir = SCSI_IOCTL_DATA_IN; break;
        case DICMOTE_SCSI_DIRECTION_OUT: dir = SCSI_IOCTL_DATA_OUT; break;
        case DICMOTE_SCSI_DIRECTION_INOUT:
        case DICMOTE_SCSI_DIRECTION_UNSPECIFIED: dir = SCSI_IOCTL_DATA_BIDIRECTIONAL; break;
        case DICMOTE_SCSI_DIRECTION_NONE:
        default: dir = SCSI_IOCTL_DATA_UNSPECIFIED; break;
    }

    memset(*sense_buffer, 0, *sense_len);

    sptd = sptd_and_sense;

    if(cdb_len > 16) cdb_len = 16;

    memcpy(&sptd->Cdb, cdb, cdb_len);
    sptd->CdbLength          = cdb_len;
    sptd->SenseInfoLength    = *sense_len;
    sptd->DataIn             = dir;
    sptd->DataTransferLength = *buf_len;
    sptd->TimeOutValue       = timeout;
    sptd->DataBuffer         = buffer;
    sptd->Length             = sizeof(SCSI_PASS_THROUGH_DIRECT);
    sptd->SenseInfoOffset    = sizeof(SCSI_PASS_THROUGH_DIRECT);

    // TODO: Timing
    hasError = !DeviceIoControl(ctx->handle,
                                IOCTL_SCSI_PASS_THROUGH_DIRECT,
                                sptd_and_sense,
                                sptd_and_sense_len,
                                sptd_and_sense,
                                sptd_and_sense_len,
                                &k,
                                NULL);

    if(hasError) error = GetLastError();

    *sense = sptd->ScsiStatus != 0;
    memcpy(*sense_buffer, (char*)sptd_and_sense + sizeof(SCSI_PASS_THROUGH_DIRECT), *sense_len);

    free(sptd_and_sense);
    return error;
}