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
#include <windows.h>

#ifndef ATA_FLAGS_DRDY_REQUIRED
#define ATA_FLAGS_DRDY_REQUIRED (1 << 0)
#endif

#ifndef ATA_FLAGS_DATA_IN
#define ATA_FLAGS_DATA_IN (1 << 1)
#endif

#ifndef ATA_FLAGS_DATA_OUT
#define ATA_FLAGS_DATA_OUT (1 << 2)
#endif

#ifndef ATA_FLAGS_USE_DMA
#define ATA_FLAGS_USE_DMA (1 << 4)
#endif

#ifndef IOCTL_ATA_PASS_THROUGH
#define IOCTL_ATA_PASS_THROUGH 0x4D02C
#endif

// TODO: Check if we can live without copying buffer in and out
int32_t Win32SendAtaChsCommand(void*                 device_ctx,
                               AtaRegistersChs       registers,
                               AtaErrorRegistersChs* error_registers,
                               uint8_t               protocol,
                               uint8_t               transfer_register,
                               char*                 buffer,
                               uint32_t              timeout,
                               uint8_t               transfer_blocks,
                               uint32_t*             duration,
                               uint32_t*             sense,
                               uint32_t*             buf_len)
{
    Win32DeviceContext*  ctx = device_ctx;
    PATA_PASS_THROUGH_EX apte;
    PVOID                apte_and_buffer;
    ULONG_PTR            offsetForBuffer;
    PCHAR                data_buffer;
    DWORD                k     = 0;
    DWORD                error = 0;
    LARGE_INTEGER        frequency;
    LARGE_INTEGER        start;
    LARGE_INTEGER        end;
    DOUBLE               interval;
    DWORD                apte_and_buffer_len;

    *duration           = 0;
    *sense              = FALSE;
    offsetForBuffer     = sizeof(ATA_PASS_THROUGH_EX) + sizeof(uint32_t);
    apte_and_buffer_len = sizeof(ATA_PASS_THROUGH_EX) + sizeof(uint32_t) + 64 * 512;

    if(!ctx) return -1;
    if(!buffer) return -1;
    if(*buf_len > 64 * 512) return -1;

    apte_and_buffer = malloc(apte_and_buffer_len);

    if(!apte_and_buffer) return -1;

    memset(apte_and_buffer, 0, apte_and_buffer_len);
    data_buffer = (PCHAR)apte_and_buffer + offsetForBuffer;
    apte        = (PATA_PASS_THROUGH_EX)apte_and_buffer;

    apte->TimeOutValue       = timeout;
    apte->DataBufferOffset   = offsetForBuffer;
    apte->Length             = sizeof(ATA_PASS_THROUGH_EX);
    apte->CurrentTaskFile[0] = registers.feature;
    apte->CurrentTaskFile[1] = registers.sector_count;
    apte->CurrentTaskFile[2] = registers.sector;
    apte->CurrentTaskFile[3] = registers.cylinder_low;
    apte->CurrentTaskFile[4] = registers.cylinder_high;
    apte->CurrentTaskFile[5] = registers.device_head;
    apte->CurrentTaskFile[6] = registers.command;

    switch(protocol)
    {
        case DICMOTE_ATA_PROTOCOL_PIO_IN:
        case DICMOTE_ATA_PROTOCOL_UDMA_IN:
        case DICMOTE_ATA_PROTOCOL_DMA: apte->AtaFlags = ATA_FLAGS_DATA_IN; break;
        case DICMOTE_ATA_PROTOCOL_PIO_OUT:
        case DICMOTE_ATA_PROTOCOL_UDMA_OUT: apte->AtaFlags = ATA_FLAGS_DATA_OUT; break;
    }

    switch(protocol)
    {
        case DICMOTE_ATA_PROTOCOL_DMA:
        case DICMOTE_ATA_PROTOCOL_DMA_QUEUED:
        case DICMOTE_ATA_PROTOCOL_FPDMA:
        case DICMOTE_ATA_PROTOCOL_UDMA_IN:
        case DICMOTE_ATA_PROTOCOL_UDMA_OUT: apte->AtaFlags |= ATA_FLAGS_USE_DMA; break;
    }

    // Unknown if needed
    apte->AtaFlags |= ATA_FLAGS_DRDY_REQUIRED;

    QueryPerformanceFrequency(&frequency);

    memcpy(data_buffer, buffer, *buf_len);

    QueryPerformanceCounter(&start);
    *sense = !DeviceIoControl(ctx->handle,
                              IOCTL_ATA_PASS_THROUGH,
                              apte_and_buffer,
                              apte_and_buffer_len,
                              apte_and_buffer,
                              apte_and_buffer_len,
                              &k,
                              NULL);
    QueryPerformanceCounter(&end);

    interval  = (DOUBLE)(end.QuadPart - start.QuadPart) / frequency.QuadPart;
    *duration = interval * 1000;

    if(*sense) error = GetLastError();

    memcpy(buffer, data_buffer, *buf_len);

    error_registers->error         = apte->CurrentTaskFile[0];
    error_registers->sector_count  = apte->CurrentTaskFile[1];
    error_registers->sector        = apte->CurrentTaskFile[2];
    error_registers->cylinder_low  = apte->CurrentTaskFile[3];
    error_registers->cylinder_high = apte->CurrentTaskFile[4];
    error_registers->device_head   = apte->CurrentTaskFile[5];
    error_registers->status        = apte->CurrentTaskFile[6];

    *sense = error_registers->error != 0 || (error_registers->status & 0xA5) != 0;

    free(apte_and_buffer);
    return error;
}

int32_t Win32SendAtaLba28Command(void*                   device_ctx,
                                 AtaRegistersLba28       registers,
                                 AtaErrorRegistersLba28* error_registers,
                                 uint8_t                 protocol,
                                 uint8_t                 transfer_register,
                                 char*                   buffer,
                                 uint32_t                timeout,
                                 uint8_t                 transfer_blocks,
                                 uint32_t*               duration,
                                 uint32_t*               sense,
                                 uint32_t*               buf_len)
{
    Win32DeviceContext*  ctx = device_ctx;
    PATA_PASS_THROUGH_EX apte;
    PVOID                apte_and_buffer;
    ULONG_PTR            offsetForBuffer;
    PCHAR                data_buffer;
    DWORD                k     = 0;
    DWORD                error = 0;
    LARGE_INTEGER        frequency;
    LARGE_INTEGER        start;
    LARGE_INTEGER        end;
    DOUBLE               interval;
    DWORD                apte_and_buffer_len;

    *duration           = 0;
    *sense              = FALSE;
    offsetForBuffer     = sizeof(ATA_PASS_THROUGH_EX) + sizeof(uint32_t);
    apte_and_buffer_len = sizeof(ATA_PASS_THROUGH_EX) + sizeof(uint32_t) + 64 * 512;

    if(!ctx) return -1;
    if(!buffer) return -1;
    if(*buf_len > 64 * 512) return -1;

    apte_and_buffer = malloc(apte_and_buffer_len);

    if(!apte_and_buffer) return -1;

    memset(apte_and_buffer, 0, apte_and_buffer_len);
    data_buffer = (PCHAR)apte_and_buffer + offsetForBuffer;
    apte        = (PATA_PASS_THROUGH_EX)apte_and_buffer;

    apte->TimeOutValue       = timeout;
    apte->DataBufferOffset   = offsetForBuffer;
    apte->Length             = sizeof(ATA_PASS_THROUGH_EX);
    apte->CurrentTaskFile[0] = registers.feature;
    apte->CurrentTaskFile[1] = registers.sector_count;
    apte->CurrentTaskFile[2] = registers.lba_low;
    apte->CurrentTaskFile[3] = registers.lba_mid;
    apte->CurrentTaskFile[4] = registers.lba_high;
    apte->CurrentTaskFile[5] = registers.device_head;
    apte->CurrentTaskFile[6] = registers.command;

    switch(protocol)
    {
        case DICMOTE_ATA_PROTOCOL_PIO_IN:
        case DICMOTE_ATA_PROTOCOL_UDMA_IN:
        case DICMOTE_ATA_PROTOCOL_DMA: apte->AtaFlags = ATA_FLAGS_DATA_IN; break;
        case DICMOTE_ATA_PROTOCOL_PIO_OUT:
        case DICMOTE_ATA_PROTOCOL_UDMA_OUT: apte->AtaFlags = ATA_FLAGS_DATA_OUT; break;
    }

    switch(protocol)
    {
        case DICMOTE_ATA_PROTOCOL_DMA:
        case DICMOTE_ATA_PROTOCOL_DMA_QUEUED:
        case DICMOTE_ATA_PROTOCOL_FPDMA:
        case DICMOTE_ATA_PROTOCOL_UDMA_IN:
        case DICMOTE_ATA_PROTOCOL_UDMA_OUT: apte->AtaFlags |= ATA_FLAGS_USE_DMA; break;
    }

    // Unknown if needed
    apte->AtaFlags |= ATA_FLAGS_DRDY_REQUIRED;

    QueryPerformanceFrequency(&frequency);

    memcpy(data_buffer, buffer, *buf_len);

    QueryPerformanceCounter(&start);
    *sense = !DeviceIoControl(ctx->handle,
                              IOCTL_ATA_PASS_THROUGH,
                              apte_and_buffer,
                              apte_and_buffer_len,
                              apte_and_buffer,
                              apte_and_buffer_len,
                              &k,
                              NULL);
    QueryPerformanceCounter(&end);

    interval  = (DOUBLE)(end.QuadPart - start.QuadPart) / frequency.QuadPart;
    *duration = interval * 1000;

    if(*sense) error = GetLastError();

    memcpy(buffer, data_buffer, *buf_len);

    error_registers->error        = apte->CurrentTaskFile[0];
    error_registers->sector_count = apte->CurrentTaskFile[1];
    error_registers->lba_low      = apte->CurrentTaskFile[2];
    error_registers->lba_mid      = apte->CurrentTaskFile[3];
    error_registers->lba_high     = apte->CurrentTaskFile[4];
    error_registers->device_head  = apte->CurrentTaskFile[5];
    error_registers->status       = apte->CurrentTaskFile[6];

    *sense = error_registers->error != 0 || (error_registers->status & 0xA5) != 0;

    free(apte_and_buffer);
    return error;
}

int32_t Win32SendAtaLba48Command(void*                   device_ctx,
                                 AtaRegistersLba48       registers,
                                 AtaErrorRegistersLba48* error_registers,
                                 uint8_t                 protocol,
                                 uint8_t                 transfer_register,
                                 char*                   buffer,
                                 uint32_t                timeout,
                                 uint8_t                 transfer_blocks,
                                 uint32_t*               duration,
                                 uint32_t*               sense,
                                 uint32_t*               buf_len)
{
    Win32DeviceContext* ctx = device_ctx;

    if(!ctx) return -1;

    // TODO: Implement
    return -1;
}