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

#include <windows.h>

#include "win32.h"
#include "../aaruremote.h"
#include "ntioctl.h"

int32_t SendScsiCommand(void*     device_ctx,
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
    DeviceContext*            ctx = device_ctx;
    PSCSI_PASS_THROUGH_DIRECT sptd;
    PVOID                     sptd_and_sense;
    UCHAR                     dir;
    DWORD                     sptd_and_sense_len;
    DWORD                     k     = 0;
    DWORD                     error = 0;
    BOOL                      hasError;
    LARGE_INTEGER             frequency;
    LARGE_INTEGER             start;
    LARGE_INTEGER             end;
    DOUBLE                    interval;

    *duration          = 0;
    *sense_len         = 32;
    sptd_and_sense_len = *sense_len + sizeof(SCSI_PASS_THROUGH_DIRECT);

    if(!ctx) return -1;

    sptd_and_sense = malloc(sptd_and_sense_len);

    if(!sptd_and_sense) return -1;

    memset(sptd_and_sense, 0, sptd_and_sense_len);

    *sense_buffer = malloc(*sense_len);

    if(!*sense_buffer)
    {
        free(sptd_and_sense);
        return -1;
    }

    switch(direction)
    {
        case AARUREMOTE_SCSI_DIRECTION_IN: dir = SCSI_IOCTL_DATA_IN; break;
        case AARUREMOTE_SCSI_DIRECTION_OUT: dir = SCSI_IOCTL_DATA_OUT; break;
        case AARUREMOTE_SCSI_DIRECTION_INOUT:
        case AARUREMOTE_SCSI_DIRECTION_UNSPECIFIED: dir = SCSI_IOCTL_DATA_BIDIRECTIONAL; break;
        case AARUREMOTE_SCSI_DIRECTION_NONE:
        default: dir = SCSI_IOCTL_DATA_UNSPECIFIED; break;
    }

    memset(*sense_buffer, 0, *sense_len);

    sptd = sptd_and_sense;

    QueryPerformanceFrequency(&frequency);

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

    QueryPerformanceCounter(&start);
    hasError = !DeviceIoControl(ctx->handle,
                                IOCTL_SCSI_PASS_THROUGH_DIRECT,
                                sptd_and_sense,
                                sptd_and_sense_len,
                                sptd_and_sense,
                                sptd_and_sense_len,
                                &k,
                                NULL);
    QueryPerformanceCounter(&end);

    interval  = (DOUBLE)(end.QuadPart - start.QuadPart) / frequency.QuadPart;
    *duration = (uint32_t)(interval * 1000.0);

    if(hasError) error = GetLastError();

    *sense = sptd->ScsiStatus != 0;
    memcpy(*sense_buffer, (char*)sptd_and_sense + sizeof(SCSI_PASS_THROUGH_DIRECT), *sense_len);

    free(sptd_and_sense);
    return error;
}