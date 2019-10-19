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

#include <malloc.h>
#include <scsi/sg.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>

int32_t LinuxSendScsiCommand(int       device_fd,
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
    sg_io_hdr_t hdr;
    int         dir, ret;
    *sense_len = 32;

    memset(&hdr, 0, sizeof(sg_io_hdr_t));
    *sense_buffer = malloc(*sense_len);

    if(!*sense_buffer) return -1;

    switch(direction)
    {
        case DICMOTE_SCSI_DIRECTION_IN: dir = SG_DXFER_FROM_DEV; break;
        case DICMOTE_SCSI_DIRECTION_OUT: dir = SG_DXFER_TO_DEV; break;
        case DICMOTE_SCSI_DIRECTION_INOUT:
        case DICMOTE_SCSI_DIRECTION_UNSPECIFIED: dir = SG_DXFER_TO_FROM_DEV; break;
        case DICMOTE_SCSI_DIRECTION_NONE:
        default: dir = SG_DXFER_NONE; break;
    }

    hdr.interface_id    = 'S';
    hdr.cmd_len         = (char)cdb_len;
    hdr.mx_sb_len       = 32;
    hdr.dxfer_direction = dir;
    hdr.dxfer_len       = *buf_len;
    hdr.dxferp          = buffer;
    hdr.cmdp            = (unsigned char*)cdb;
    hdr.sbp             = (unsigned char*)*sense_buffer;
    hdr.timeout         = timeout;
    hdr.flags           = SG_FLAG_DIRECT_IO;

    ret = ioctl(device_fd, SG_IO, &hdr);

    *sense = (hdr.info & SG_INFO_OK_MASK) != SG_INFO_OK;
    // TODO: Manual timing if duration is 0
    *duration  = hdr.duration;
    *sense_len = hdr.sb_len_wr;

    return ret; // TODO: Implement
}