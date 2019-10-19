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

#include "linux.h"

#include <scsi/sg.h>
#include <stddef.h>
#include <string.h>

int32_t ata_protocol_to_scsi_direction(uint8_t protocol)
{
    switch(protocol)
    {
        case DICMOTE_ATA_PROTOCOL_DEVICE_DIAGNOSTIC:
        case DICMOTE_ATA_PROTOCOL_DEVICE_RESET:
        case DICMOTE_ATA_PROTOCOL_HARD_RESET:
        case DICMOTE_ATA_PROTOCOL_NO_DATA:
        case DICMOTE_ATA_PROTOCOL_SOFT_RESET:
        case DICMOTE_ATA_PROTOCOL_RETURN_RESPONSE: return DICMOTE_SCSI_DIRECTION_NONE;
        case DICMOTE_ATA_PROTOCOL_PIO_IN:
        case DICMOTE_ATA_PROTOCOL_UDMA_IN: return DICMOTE_SCSI_DIRECTION_IN;
        case DICMOTE_ATA_PROTOCOL_PIO_OUT:
        case DICMOTE_ATA_PROTOCOL_UDMA_OUT: return DICMOTE_SCSI_DIRECTION_OUT;
        default: return DICMOTE_SCSI_DIRECTION_UNSPECIFIED;
    }
}

int32_t linux_send_ata_chs_command(int                   device_fd,
                                   AtaRegistersChs       registers,
                                   AtaErrorRegistersChs* errorRegisters,
                                   uint8_t               protocol,
                                   uint8_t               transferRegister,
                                   char*                 buffer,
                                   uint32_t              timeout,
                                   uint8_t               transferBlocks,
                                   uint32_t*             duration,
                                   uint32_t*             sense,
                                   uint32_t*             buf_len)
{
    duration = 0;
    sense    = 0;
    unsigned char cdb[16];
    char*         sense_buf;
    uint32_t      sense_len;

    memset(&cdb, 0, 16);

    cdb[0] = 0x85;
    cdb[1] = (protocol << 1) & 0x1E;
    if(transferRegister != DICMOTE_ATA_TRANSFER_REGISTER_NONE && protocol != DICMOTE_ATA_PROTOCOL_NO_DATA)
    {
        switch(protocol)
        {
            case DICMOTE_ATA_PROTOCOL_PIO_IN:
            case DICMOTE_ATA_PROTOCOL_UDMA_IN: cdb[2] = 0x08; break;
            default: cdb[2] = 0x00; break;
        }

        if(transferBlocks) cdb[2] |= 0x04;

        cdb[2] |= (transferRegister & 0x03);
    }

    cdb[4]  = registers.Feature;
    cdb[6]  = registers.SectorCount;
    cdb[8]  = registers.Sector;
    cdb[10] = registers.CylinderLow;
    cdb[12] = registers.CylinderHigh;
    cdb[13] = registers.DeviceHead;
    cdb[14] = registers.Command;

    int error = linux_send_scsi_command(device_fd,
                                        (char*)cdb,
                                        buffer,
                                        &sense_buf,
                                        timeout,
                                        ata_protocol_to_scsi_direction(protocol),
                                        duration,
                                        sense,
                                        16,
                                        buf_len,
                                        &sense_len);

    if(sense_len < 22 || (sense_buf[8] != 0x09 && sense_buf[9] != 0x0C)) return error;

    errorRegisters->Error = sense_buf[11];

    errorRegisters->SectorCount  = sense_buf[13];
    errorRegisters->Sector       = sense_buf[15];
    errorRegisters->CylinderLow  = sense_buf[17];
    errorRegisters->CylinderHigh = sense_buf[19];
    errorRegisters->DeviceHead   = sense_buf[20];
    errorRegisters->Status       = sense_buf[21];

    *sense = errorRegisters->Error != 0 || (errorRegisters->Status & 0xA5) != 0;

    return error;
}

int32_t linux_send_ata_lba28_command(int                     device_fd,
                                     AtaRegistersLba28       registers,
                                     AtaErrorRegistersLba28* errorRegisters,
                                     uint8_t                 protocol,
                                     uint8_t                 transferRegister,
                                     char*                   buffer,
                                     uint32_t                timeout,
                                     uint8_t                 transferBlocks,
                                     uint32_t*               duration,
                                     uint32_t*               sense,
                                     uint32_t*               buf_len)
{
    duration = 0;
    sense    = 0;
    unsigned char cdb[16];
    char*         sense_buf;
    uint32_t      sense_len;

    memset(&cdb, 0, 16);

    cdb[0] = 0x85;
    cdb[1] = (protocol << 1) & 0x1E;
    if(transferRegister != DICMOTE_ATA_TRANSFER_REGISTER_NONE && protocol != DICMOTE_ATA_PROTOCOL_NO_DATA)
    {
        switch(protocol)
        {
            case DICMOTE_ATA_PROTOCOL_PIO_IN:
            case DICMOTE_ATA_PROTOCOL_UDMA_IN: cdb[2] = 0x08; break;
            default: cdb[2] = 0x00; break;
        }

        if(transferBlocks) cdb[2] |= 0x04;

        cdb[2] |= (transferRegister & 0x03);
    }

    cdb[2] |= 0x20;

    cdb[4]  = registers.Feature;
    cdb[6]  = registers.SectorCount;
    cdb[8]  = registers.LbaLow;
    cdb[10] = registers.LbaMid;
    cdb[12] = registers.LbaHigh;
    cdb[13] = registers.DeviceHead;
    cdb[14] = registers.Command;

    int error = linux_send_scsi_command(device_fd,
                                        (char*)cdb,
                                        buffer,
                                        &sense_buf,
                                        timeout,
                                        ata_protocol_to_scsi_direction(protocol),
                                        duration,
                                        sense,
                                        16,
                                        buf_len,
                                        &sense_len);

    if(sense_len < 22 || (sense_buf[8] != 0x09 && sense_buf[9] != 0x0C)) return error;

    errorRegisters->Error = sense_buf[11];

    errorRegisters->SectorCount = sense_buf[13];
    errorRegisters->LbaLow      = sense_buf[15];
    errorRegisters->LbaMid      = sense_buf[17];
    errorRegisters->LbaHigh     = sense_buf[19];
    errorRegisters->DeviceHead  = sense_buf[20];
    errorRegisters->Status      = sense_buf[21];

    *sense = errorRegisters->Error != 0 || (errorRegisters->Status & 0xA5) != 0;

    return error;
}
int32_t linux_send_ata_lba48_command(int                     device_fd,
                                     AtaRegistersLba48       registers,
                                     AtaErrorRegistersLba48* errorRegisters,
                                     uint8_t                 protocol,
                                     uint8_t                 transferRegister,
                                     char*                   buffer,
                                     uint32_t                timeout,
                                     uint8_t                 transferBlocks,
                                     uint32_t*               duration,
                                     uint32_t*               sense,
                                     uint32_t*               buf_len)
{
    duration = 0;
    sense    = 0;
    unsigned char cdb[16];
    char*         sense_buf;
    uint32_t      sense_len;

    memset(&cdb, 0, 16);

    cdb[0] = 0x85;
    cdb[1] = (protocol << 1) & 0x1E;
    cdb[1] |= 0x01;
    if(transferRegister != DICMOTE_ATA_TRANSFER_REGISTER_NONE && protocol != DICMOTE_ATA_PROTOCOL_NO_DATA)
    {
        switch(protocol)
        {
            case DICMOTE_ATA_PROTOCOL_PIO_IN:
            case DICMOTE_ATA_PROTOCOL_UDMA_IN: cdb[2] = 0x08; break;
            default: cdb[2] = 0x00; break;
        }

        if(transferBlocks) cdb[2] |= 0x04;

        cdb[2] |= (transferRegister & 0x03);
    }

    cdb[2] |= 0x20;

    cdb[3]  = ((registers.Feature & 0xFF00) >> 8);
    cdb[4]  = (registers.Feature & 0xFF);
    cdb[5]  = ((registers.SectorCount & 0xFF00) >> 8);
    cdb[6]  = (registers.SectorCount & 0xFF);
    cdb[7]  = ((registers.LbaLow & 0xFF00) >> 8);
    cdb[8]  = (registers.LbaLow & 0xFF);
    cdb[9]  = ((registers.LbaMid & 0xFF00) >> 8);
    cdb[10] = (registers.LbaMid & 0xFF);
    cdb[11] = ((registers.LbaHigh & 0xFF00) >> 8);
    cdb[12] = (registers.LbaHigh & 0xFF);
    cdb[13] = registers.DeviceHead;
    cdb[14] = registers.Command;

    int error = linux_send_scsi_command(device_fd,
                                        (char*)cdb,
                                        buffer,
                                        &sense_buf,
                                        timeout,
                                        ata_protocol_to_scsi_direction(protocol),
                                        duration,
                                        sense,
                                        16,
                                        buf_len,
                                        &sense_len);

    if(sense_len < 22 || (sense_buf[8] != 0x09 && sense_buf[9] != 0x0C)) return error;

    errorRegisters->Error = sense_buf[11];

    errorRegisters->SectorCount = (uint16_t)((sense_buf[12] << 8) + sense_buf[13]);
    errorRegisters->LbaLow      = (uint16_t)((sense_buf[14] << 8) + sense_buf[15]);
    errorRegisters->LbaMid      = (uint16_t)((sense_buf[16] << 8) + sense_buf[17]);
    errorRegisters->LbaHigh     = (uint16_t)((sense_buf[18] << 8) + sense_buf[19]);
    errorRegisters->DeviceHead  = sense_buf[20];
    errorRegisters->Status      = sense_buf[21];

    *sense = errorRegisters->Error != 0 || (errorRegisters->Status & 0xA5) != 0;

    return error;
}