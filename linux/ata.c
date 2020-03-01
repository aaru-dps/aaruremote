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

#include "linux.h"

#include <string.h>

int32_t AtaProtocolToScsiDirection(uint8_t protocol)
{
    switch(protocol)
    {
        case AARUREMOTE_ATA_PROTOCOL_DEVICE_DIAGNOSTIC:
        case AARUREMOTE_ATA_PROTOCOL_DEVICE_RESET:
        case AARUREMOTE_ATA_PROTOCOL_HARD_RESET:
        case AARUREMOTE_ATA_PROTOCOL_NO_DATA:
        case AARUREMOTE_ATA_PROTOCOL_SOFT_RESET:
        case AARUREMOTE_ATA_PROTOCOL_RETURN_RESPONSE: return AARUREMOTE_SCSI_DIRECTION_NONE;
        case AARUREMOTE_ATA_PROTOCOL_PIO_IN:
        case AARUREMOTE_ATA_PROTOCOL_UDMA_IN: return AARUREMOTE_SCSI_DIRECTION_IN;
        case AARUREMOTE_ATA_PROTOCOL_PIO_OUT:
        case AARUREMOTE_ATA_PROTOCOL_UDMA_OUT: return AARUREMOTE_SCSI_DIRECTION_OUT;
        default: return AARUREMOTE_SCSI_DIRECTION_UNSPECIFIED;
    }
}

int32_t LinuxSendAtaChsCommand(void*                 device_ctx,
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
    *duration = 0;
    *sense    = 0;
    unsigned char       cdb[16];
    char*               sense_buf;
    uint32_t            sense_len;
    LinuxDeviceContext* ctx = device_ctx;

    if(!ctx) return -1;

    memset(&cdb, 0, 16);

    cdb[0] = 0x85;
    cdb[1] = (protocol << 1) & 0x1E;
    if(transfer_register != AARUREMOTE_ATA_TRANSFER_REGISTER_NONE && protocol != AARUREMOTE_ATA_PROTOCOL_NO_DATA)
    {
        switch(protocol)
        {
            case AARUREMOTE_ATA_PROTOCOL_PIO_IN:
            case AARUREMOTE_ATA_PROTOCOL_UDMA_IN: cdb[2] = 0x08; break;
            default: cdb[2] = 0x00; break;
        }

        if(transfer_blocks) cdb[2] |= 0x04;

        cdb[2] |= (transfer_register & 0x03);
    }

    cdb[4]  = registers.feature;
    cdb[6]  = registers.sector_count;
    cdb[8]  = registers.sector;
    cdb[10] = registers.cylinder_low;
    cdb[12] = registers.cylinder_high;
    cdb[13] = registers.device_head;
    cdb[14] = registers.command;

    int error = LinuxSendScsiCommand(ctx,
                                     (char*)cdb,
                                     buffer,
                                     &sense_buf,
                                     timeout,
                                     AtaProtocolToScsiDirection(protocol),
                                     duration,
                                     sense,
                                     16,
                                     buf_len,
                                     &sense_len);

    if(sense_len < 22 || (sense_buf[8] != 0x09 && sense_buf[9] != 0x0C)) return error;

    error_registers->error = sense_buf[11];

    error_registers->sector_count  = sense_buf[13];
    error_registers->sector        = sense_buf[15];
    error_registers->cylinder_low  = sense_buf[17];
    error_registers->cylinder_high = sense_buf[19];
    error_registers->device_head   = sense_buf[20];
    error_registers->status        = sense_buf[21];

    *sense = error_registers->error != 0 || (error_registers->status & 0xA5) != 0;

    return error;
}

int32_t LinuxSendAtaLba28Command(void*                   device_ctx,
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
    *duration = 0;
    *sense    = 0;
    unsigned char       cdb[16];
    char*               sense_buf;
    uint32_t            sense_len;
    LinuxDeviceContext* ctx = device_ctx;

    if(!ctx) return -1;

    memset(&cdb, 0, 16);

    cdb[0] = 0x85;
    cdb[1] = (protocol << 1) & 0x1E;
    if(transfer_register != AARUREMOTE_ATA_TRANSFER_REGISTER_NONE && protocol != AARUREMOTE_ATA_PROTOCOL_NO_DATA)
    {
        switch(protocol)
        {
            case AARUREMOTE_ATA_PROTOCOL_PIO_IN:
            case AARUREMOTE_ATA_PROTOCOL_UDMA_IN: cdb[2] = 0x08; break;
            default: cdb[2] = 0x00; break;
        }

        if(transfer_blocks) cdb[2] |= 0x04;

        cdb[2] |= (transfer_register & 0x03);
    }

    cdb[2] |= 0x20;

    cdb[4]  = registers.feature;
    cdb[6]  = registers.sector_count;
    cdb[8]  = registers.lba_low;
    cdb[10] = registers.lba_mid;
    cdb[12] = registers.lba_high;
    cdb[13] = registers.device_head;
    cdb[14] = registers.command;

    int error = LinuxSendScsiCommand(ctx,
                                     (char*)cdb,
                                     buffer,
                                     &sense_buf,
                                     timeout,
                                     AtaProtocolToScsiDirection(protocol),
                                     duration,
                                     sense,
                                     16,
                                     buf_len,
                                     &sense_len);

    if(sense_len < 22 || (sense_buf[8] != 0x09 && sense_buf[9] != 0x0C)) return error;

    error_registers->error = sense_buf[11];

    error_registers->sector_count = sense_buf[13];
    error_registers->lba_low      = sense_buf[15];
    error_registers->lba_mid      = sense_buf[17];
    error_registers->lba_high     = sense_buf[19];
    error_registers->device_head  = sense_buf[20];
    error_registers->status       = sense_buf[21];

    *sense = error_registers->error != 0 || (error_registers->status & 0xA5) != 0;

    return error;
}

int32_t LinuxSendAtaLba48Command(void*                   device_ctx,
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
    *duration = 0;
    *sense    = 0;
    unsigned char       cdb[16];
    char*               sense_buf;
    uint32_t            sense_len;
    LinuxDeviceContext* ctx = device_ctx;

    if(!ctx) return -1;

    memset(&cdb, 0, 16);

    cdb[0] = 0x85;
    cdb[1] = (protocol << 1) & 0x1E;
    cdb[1] |= 0x01;
    if(transfer_register != AARUREMOTE_ATA_TRANSFER_REGISTER_NONE && protocol != AARUREMOTE_ATA_PROTOCOL_NO_DATA)
    {
        switch(protocol)
        {
            case AARUREMOTE_ATA_PROTOCOL_PIO_IN:
            case AARUREMOTE_ATA_PROTOCOL_UDMA_IN: cdb[2] = 0x08; break;
            default: cdb[2] = 0x00; break;
        }

        if(transfer_blocks) cdb[2] |= 0x04;

        cdb[2] |= (transfer_register & 0x03);
    }

    cdb[2] |= 0x20;

    cdb[3]  = ((registers.feature & 0xFF00) >> 8);
    cdb[4]  = (registers.feature & 0xFF);
    cdb[5]  = ((registers.sector_count & 0xFF00) >> 8);
    cdb[6]  = (registers.sector_count & 0xFF);
    cdb[7]  = ((registers.lba_low & 0xFF00) >> 8);
    cdb[8]  = (registers.lba_low & 0xFF);
    cdb[9]  = ((registers.lba_mid & 0xFF00) >> 8);
    cdb[10] = (registers.lba_mid & 0xFF);
    cdb[11] = ((registers.lba_high & 0xFF00) >> 8);
    cdb[12] = (registers.lba_high & 0xFF);
    cdb[13] = registers.device_head;
    cdb[14] = registers.command;

    int error = LinuxSendScsiCommand(ctx,
                                     (char*)cdb,
                                     buffer,
                                     &sense_buf,
                                     timeout,
                                     AtaProtocolToScsiDirection(protocol),
                                     duration,
                                     sense,
                                     16,
                                     buf_len,
                                     &sense_len);

    if(sense_len < 22 || (sense_buf[8] != 0x09 && sense_buf[9] != 0x0C)) return error;

    error_registers->error = sense_buf[11];

    error_registers->sector_count = (uint16_t)((sense_buf[12] << 8) + sense_buf[13]);
    error_registers->lba_low      = (uint16_t)((sense_buf[14] << 8) + sense_buf[15]);
    error_registers->lba_mid      = (uint16_t)((sense_buf[16] << 8) + sense_buf[17]);
    error_registers->lba_high     = (uint16_t)((sense_buf[18] << 8) + sense_buf[19]);
    error_registers->device_head  = sense_buf[20];
    error_registers->status       = sense_buf[21];

    *sense = error_registers->error != 0 || (error_registers->status & 0xA5) != 0;

    return error;
}