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

#include "dicmote.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    AtaErrorRegistersChs           ata_chs_error_regs;
    AtaErrorRegistersLba28         ata_lba28_error_regs;
    AtaErrorRegistersLba48         ata_lba48_error_regs;
    char                           device_path[1024];
    char*                          buffer;
    char*                          cdb_buf;
    char*                          cid;
    char*                          csd;
    char*                          in_buf;
    char*                          ocr;
    char*                          out_buf;
    char*                          scr;
    char*                          sense_buf;
    DicPacketCmdAtaChs*            pkt_cmd_ata_chs;
    DicPacketCmdAtaLba28*          pkt_cmd_ata_lba28;
    DicPacketCmdAtaLba48*          pkt_cmd_ata_lba48;
    DicPacketCmdOpen*              pkt_dev_open;
    DicPacketCmdScsi*              pkt_cmd_scsi;
    DicPacketCmdSdhci*             pkt_cmd_sdhci;
    DicPacketHeader*               pkt_hdr;
    DicPacketHello*                pkt_server_hello;
    DicPacketHello*                pkt_client_hello;
    DicPacketNop*                  pkt_nop;
    DicPacketResAtaChs*            pkt_res_ata_chs;
    DicPacketResAtaLba28*          pkt_res_ata_lba28;
    DicPacketResAtaLba48*          pkt_res_ata_lba48;
    DicPacketResGetDeviceType*     pkt_dev_type;
    DicPacketResGetFireWireData*   pkt_res_firewire;
    DicPacketResGetPcmciaData*     pkt_res_pcmcia;
    DicPacketResGetSdhciRegisters* pkt_res_sdhci_registers;
    DicPacketResGetUsbData*        pkt_res_usb;
    DicPacketResListDevs*          pkt_res_devinfo;
    DicPacketResScsi*              pkt_res_scsi;
    DicPacketResSdhci*             pkt_res_sdhci;
    int                            device_fd = -1;
    int                            skip_next_hdr;
    int                            cli_sock, sock_fd;
    int                            ret;
    socklen_t                      cli_len;
    ssize_t                        recv_size;
    struct DeviceInfoList*         device_info_list;
    struct sockaddr_in             cli_addr, serv_addr;
    uint32_t                       duration;
    uint32_t                       sdhci_response[4];
    uint32_t                       sense;
    uint32_t                       sense_len;
    uint64_t                       n;

    Initialize();

    printf("DiscImageChef Remote Server %s\n", DICMOTE_VERSION);
    printf("Copyright (C) 2019 Natalia Portillo\n");

    pkt_server_hello = GetHello();

    if(!pkt_server_hello)
    {
        printf("Error %d getting system version.\n", errno);
        return 1;
    }

    printf(
        "Running under %s %s (%s).\n", pkt_server_hello->sysname, pkt_server_hello->release, pkt_server_hello->machine);

    ret = PrintNetworkAddresses();

    if(ret)
    {
        printf("Error %d enumerating interfaces\n", errno);
        return 1;
    }

    PlatformLoop(pkt_server_hello);
}