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

#include "ntioctl.h"
#include "win32.h"

#include <stdint.h>

int32_t Win32SendSdhciCommand(void*     device_ctx,
                              uint8_t   command,
                              uint8_t   write,
                              uint8_t   application,
                              uint32_t  flags,
                              uint32_t  argument,
                              uint32_t  block_size,
                              uint32_t  blocks,
                              char*     buffer,
                              uint32_t  timeout,
                              uint32_t* response,
                              uint32_t* duration,
                              uint32_t* sense)
{
    Win32DeviceContext*          ctx = device_ctx;
    DWORD                        cmdbuf_len;
    PCHAR                        cmdbuf;
    DWORD                        buf_len;
    PSFFDISK_DEVICE_COMMAND_DATA cmd_data;
    PSDCMD_DESCRIPTOR            cmd_descriptor;
    PCHAR                        data_buf;
    DWORD                        error = 0;
    DWORD                        k     = 0;
    LARGE_INTEGER                frequency;
    LARGE_INTEGER                start;
    LARGE_INTEGER                end;
    DOUBLE                       interval;

    if(!ctx) return -1;

    buf_len    = blocks * block_size;
    cmdbuf_len = sizeof(SFFDISK_DEVICE_COMMAND_DATA) + sizeof(SDCMD_DESCRIPTOR) + buf_len;

    cmdbuf = malloc(cmdbuf_len);

    if(!cmdbuf) return -1;

    memset(cmdbuf, 0, cmdbuf_len);
    cmd_data       = (PSFFDISK_DEVICE_COMMAND_DATA)cmdbuf;
    cmd_descriptor = (PSDCMD_DESCRIPTOR)(cmdbuf + sizeof(SFFDISK_DEVICE_COMMAND_DATA));
    data_buf       = cmdbuf + sizeof(SFFDISK_DEVICE_COMMAND_DATA) + sizeof(SDCMD_DESCRIPTOR);

    memcpy(data_buf, buffer, buf_len);

    cmd_data->HeaderSize              = sizeof(SFFDISK_DEVICE_COMMAND_DATA);
    cmd_data->Command                 = SFFDISK_DC_DEVICE_COMMAND;
    cmd_data->ProtocolArgumentSize    = sizeof(SDCMD_DESCRIPTOR);
    cmd_data->DeviceDataBufferSize    = buf_len;
    cmd_descriptor->Cmd               = command;
    cmd_descriptor->CmdClass          = application ? SDCC_APP_CMD : SDCC_STANDARD;
    cmd_descriptor->TransferDirection = write ? SDTD_WRITE : SDTD_READ;
    cmd_descriptor->TransferType      = (flags & DICMOTE_MMC_COMMAND_ADTC) ? SDTT_SINGLE_BLOCK : SDTT_CMD_ONLY;
    cmd_descriptor->ResponseType      = 0;

    if((flags & DICMOTE_MMC_RESPONSE_R1) || (flags & DICMOTE_MMC_RESPONSE_SPI_R1))
        cmd_descriptor->ResponseType = SDRT_1;
    if((flags & DICMOTE_MMC_RESPONSE_R1B) || (flags & DICMOTE_MMC_RESPONSE_SPI_R1B))
        cmd_descriptor->ResponseType = SDRT_1B;
    if((flags & DICMOTE_MMC_RESPONSE_R2) || (flags & DICMOTE_MMC_RESPONSE_SPI_R2))
        cmd_descriptor->ResponseType = SDRT_2;
    if((flags & DICMOTE_MMC_RESPONSE_R3) || (flags & DICMOTE_MMC_RESPONSE_SPI_R3))
        cmd_descriptor->ResponseType = SDRT_3;
    if((flags & DICMOTE_MMC_RESPONSE_R4) || (flags & DICMOTE_MMC_RESPONSE_SPI_R4))
        cmd_descriptor->ResponseType = SDRT_4;
    if((flags & DICMOTE_MMC_RESPONSE_R5) || (flags & DICMOTE_MMC_RESPONSE_SPI_R5))
        cmd_descriptor->ResponseType = SDRT_5;
    if((flags & DICMOTE_MMC_RESPONSE_R6)) cmd_descriptor->ResponseType = SDRT_6;

    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&start);
    *sense =
        !DeviceIoControl(ctx->handle, IOCTL_SFFDISK_DEVICE_COMMAND, cmdbuf, cmdbuf_len, cmdbuf, cmdbuf_len, &k, NULL);
    QueryPerformanceCounter(&end);

    interval  = (DOUBLE)(end.QuadPart - start.QuadPart) / frequency.QuadPart;
    *duration = interval * 1000;

    if(*sense) error = GetLastError();

    memcpy(buffer, data_buf, buf_len);

    free(cmdbuf);
    return error;
}

BOOL GuidEquals(GUID a, GUID b) { return memcmp(&a, &b, 16) == 0; }

BOOL IsSdhci(HANDLE handle)
{
    SFFDISK_QUERY_DEVICE_PROTOCOL_DATA query;
    DWORD                              k       = 0;
    GUID                               sdGuid  = GUID_SFF_PROTOCOL_SD;
    GUID                               mmcGuid = GUID_SFF_PROTOCOL_MMC;

    DeviceIoControl(handle,
                    IOCTL_SFFDISK_QUERY_DEVICE_PROTOCOL,
                    NULL,
                    0,
                    &query,
                    sizeof(SFFDISK_QUERY_DEVICE_PROTOCOL_DATA),
                    &k,
                    NULL);

    return GuidEquals(query.ProtocolGUID, sdGuid) || GuidEquals(query.ProtocolGUID, mmcGuid);
}

int32_t Win32GetSdhciRegisters(void*     device_ctx,
                               char**    csd,
                               char**    cid,
                               char**    ocr,
                               char**    scr,
                               uint32_t* csd_len,
                               uint32_t* cid_len,
                               uint32_t* ocr_len,
                               uint32_t* scr_len)
{
    Win32DeviceContext* ctx = device_ctx;
    uint32_t            duration;
    uint32_t            sense;

    if(!ctx) return -1;

    if(!IsSdhci(ctx->handle)) return -1;

    *csd = malloc(16);
    if(*csd)
    {
        *csd_len = 16;
        Win32SendSdhciCommand(device_ctx,
                              9,
                              0,
                              0,
                              DICMOTE_MMC_RESPONSE_SPI_R2 | DICMOTE_MMC_RESPONSE_R2 | DICMOTE_MMC_COMMAND_AC,
                              0,
                              16,
                              1,
                              *csd,
                              1000,
                              NULL,
                              &duration,
                              &sense);

        if(sense)
        {
            *csd_len = 0;
            free(*csd);
        }
    }

    *cid = malloc(16);
    if(*cid)
    {
        *cid_len = 16;
        Win32SendSdhciCommand(device_ctx,
                              10,
                              0,
                              0,
                              DICMOTE_MMC_RESPONSE_SPI_R2 | DICMOTE_MMC_RESPONSE_R2 | DICMOTE_MMC_COMMAND_AC,
                              0,
                              16,
                              1,
                              *cid,
                              1000,
                              NULL,
                              &duration,
                              &sense);

        if(sense)
        {
            *cid_len = 0;
            free(*cid);
        }
    }

    *scr = malloc(8);
    if(*scr)
    {
        *scr_len = 8;
        Win32SendSdhciCommand(device_ctx,
                              10,
                              0,
                              0,
                              DICMOTE_MMC_RESPONSE_SPI_R1 | DICMOTE_MMC_RESPONSE_R1 | DICMOTE_MMC_COMMAND_ADTC,
                              0,
                              8,
                              1,
                              *scr,
                              1000,
                              NULL,
                              &duration,
                              &sense);

        if(sense)
        {
            *scr_len = 0;
            free(*scr);
        }
    }

    *ocr = malloc(4);
    if(*ocr)
    {
        *ocr_len = 4;
        Win32SendSdhciCommand(device_ctx,
                              *scr_len > 0 ? 41 : 1,
                              0,
                              0,
                              DICMOTE_MMC_RESPONSE_SPI_R1 | DICMOTE_MMC_RESPONSE_R1 | DICMOTE_MMC_COMMAND_ADTC,
                              0,
                              4,
                              1,
                              *ocr,
                              1000,
                              NULL,
                              &duration,
                              &sense);

        if(sense)
        {
            *ocr_len = 0;
            free(*ocr);
        }
    }

    return *csd_len > 0 || *cid_len > 0 || *scr_len > 0 || *ocr_len > 0;
}