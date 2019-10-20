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
#include <libnet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>

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
    char                           ipv4_address[INET_ADDRSTRLEN];
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

    printf("Opening socket.\n");
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(sock_fd < 0)
    {
        printf("Error %d opening socket.\n", errno);
        return 1;
    }

    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port        = htons(DICMOTE_PORT);

    if(bind(sock_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("Error %d binding socket.\n", errno);
        close(sock_fd);
        return 1;
    }

    ret = PrintNetworkAddresses();

    if(ret)
    {
        printf("Error %d enumerating interfaces\n", errno);
        return 1;
    }

    ret = listen(sock_fd, 1);

    if(ret)
    {
        printf("Error %d listening.\n", errno);
        close(sock_fd);
        return 1;
    }

    pkt_nop = malloc(sizeof(DicPacketNop));

    if(!pkt_nop)
    {
        printf("Fatal error %d allocating memory.\n", errno);
        close(sock_fd);
        return 1;
    }

    memset(pkt_nop, 0, sizeof(DicPacketNop));

    pkt_nop->hdr.id          = DICMOTE_PACKET_ID;
    pkt_nop->hdr.len         = sizeof(DicPacketNop);
    pkt_nop->hdr.version     = DICMOTE_PACKET_VERSION;
    pkt_nop->hdr.packet_type = DICMOTE_PACKET_TYPE_NOP;

    for(;;)
    {
        printf("\n");
        printf("Waiting for a client...\n");

        cli_len  = sizeof(cli_addr);
        cli_sock = accept(sock_fd, (struct sockaddr*)&cli_addr, &cli_len);

        if(cli_sock < 0)
        {
            printf("Error %d accepting incoming connection.\n", errno);
            close(sock_fd);
            return 1;
        }

        inet_ntop(AF_INET, &cli_addr.sin_addr, ipv4_address, INET_ADDRSTRLEN);
        printf("Client %s connected successfully.\n", ipv4_address);

        write(cli_sock, pkt_server_hello, sizeof(DicPacketHello));

        pkt_hdr = malloc(sizeof(DicPacketHeader));

        if(!pkt_hdr)
        {
            printf("Fatal error %d allocating memory.\n", errno);
            close(cli_sock);
            close(sock_fd);
            free(pkt_server_hello);
            return 1;
        }

        recv_size = recv(cli_sock, pkt_hdr, sizeof(DicPacketHeader), MSG_PEEK);

        if(recv_size < 0)
        {
            printf("Error %d reading response from client.\n", errno);
            free(pkt_hdr);
            close(cli_sock);
            continue;
        }

        if(recv_size == 0)
        {
            printf("Client closed connection.\n");
            free(pkt_hdr);
            close(cli_sock);
            continue;
        }

        if(pkt_hdr->id != DICMOTE_PACKET_ID)
        {
            printf("Received data is not a correct dicremote packet, closing connection...\n");
            free(pkt_hdr);
            close(cli_sock);
            continue;
        }

        if(pkt_hdr->version != DICMOTE_PACKET_VERSION)
        {
            printf("Unrecognized packet version, closing connection...\n");
            free(pkt_hdr);
            close(cli_sock);
            continue;
        }

        if(pkt_hdr->packet_type != DICMOTE_PACKET_TYPE_HELLO)
        {
            printf("Expecting hello packet type, received type %d, closing connection...\n", pkt_hdr->packet_type);
            free(pkt_hdr);
            close(cli_sock);
            continue;
        }

        pkt_client_hello = malloc(pkt_hdr->len);

        if(!pkt_client_hello)
        {
            printf("Fatal error %d allocating memory for packet, closing connection...\n", errno);
            free(pkt_hdr);
            close(cli_sock);
            continue;
        }

        recv_size = recv(cli_sock, pkt_client_hello, pkt_hdr->len, 0);

        if(recv_size != pkt_hdr->len)
        {
            printf("Expected %d bytes of packet, got %ld, closing connection...\n", pkt_hdr->len, recv_size);
            close(cli_sock);
            free(pkt_hdr);
            free(pkt_client_hello);
            continue;
        }

        printf("Client application: %s %s\n", pkt_client_hello->application, pkt_client_hello->version);
        printf("Client operating system: %s %s (%s)\n",
               pkt_client_hello->sysname,
               pkt_client_hello->release,
               pkt_client_hello->machine);
        printf("Client maximum protocol: %d\n", pkt_client_hello->max_protocol);

        free(pkt_client_hello);

        skip_next_hdr = 0;

        for(;;)
        {
            if(skip_next_hdr)
            {
                in_buf = malloc(pkt_hdr->len);

                if(!in_buf)
                {
                    printf("Fatal error %d allocating memory for packet, closing connection...\n", errno);
                    free(pkt_hdr);
                    close(cli_sock);
                    continue;
                }

                recv(cli_sock, in_buf, pkt_hdr->len, 0);
                free(in_buf);
                skip_next_hdr = 0;
            }

            recv_size = recv(cli_sock, pkt_hdr, sizeof(DicPacketHeader), MSG_PEEK);

            if(recv_size < 0)
            {
                printf("Error %d reading response from client, closing connection...\n", errno);
                close(cli_sock);
                free(pkt_hdr);
                break;
            }

            if(recv_size == 0)
            {
                printf("Client closed connection, closing connection...\n");
                close(cli_sock);
                free(pkt_hdr);
                break;
            }

            if(pkt_hdr->id != DICMOTE_PACKET_ID)
            {
                printf("Received data is not a correct dicremote packet, closing connection...\n");
                close(cli_sock);
                free(pkt_hdr);
                break;
            }

            if(pkt_hdr->version != DICMOTE_PACKET_VERSION)
            {
                printf("Unrecognized packet version, skipping...\n");
                skip_next_hdr = 1;
                continue;
            }

            switch(pkt_hdr->packet_type)
            {
                case DICMOTE_PACKET_TYPE_HELLO:
                    pkt_nop->reason_code = DICMOTE_PACKET_NOP_REASON_OOO;
                    memset(&pkt_nop->reason, 0, 256);
                    strncpy(pkt_nop->reason, "Received hello packet out of order, skipping...", 256);
                    write(cli_sock, pkt_nop, sizeof(DicPacketNop));
                    printf("%s...\n", pkt_nop->reason);
                    skip_next_hdr = 1;
                    continue;
                case DICMOTE_PACKET_TYPE_COMMAND_LIST_DEVICES:
                    device_info_list = ListDevices();

                    // Packet only contains header so, dummy
                    in_buf = malloc(pkt_hdr->len);

                    if(!in_buf)
                    {
                        printf("Fatal error %d allocating memory for packet, closing connection...\n", errno);
                        free(pkt_hdr);
                        close(cli_sock);
                        continue;
                    }

                    recv(cli_sock, in_buf, pkt_hdr->len, 0);
                    free(in_buf);

                    if(!device_info_list)
                    {
                        pkt_nop->reason_code = DICMOTE_PACKET_NOP_REASON_ERROR_LIST_DEVICES;
                        memset(&pkt_nop->reason, 0, 256);
                        strncpy(pkt_nop->reason, "Could not get device list, continuing...", 256);
                        write(cli_sock, pkt_nop, sizeof(DicPacketNop));
                        printf("%s...\n", pkt_nop->reason);
                        continue;
                    }

                    pkt_res_devinfo          = malloc(sizeof(DicPacketResListDevs));
                    pkt_res_devinfo->devices = DeviceInfoListCount(device_info_list);

                    n      = sizeof(DicPacketResListDevs) + pkt_res_devinfo->devices * sizeof(DeviceInfo);
                    in_buf = malloc(n);
                    ((DicPacketResListDevs*)in_buf)->hdr.len = n;
                    ((DicPacketResListDevs*)in_buf)->devices = pkt_res_devinfo->devices;
                    free(pkt_res_devinfo);
                    pkt_res_devinfo = (DicPacketResListDevs*)in_buf;

                    pkt_res_devinfo->hdr.id          = DICMOTE_PACKET_ID;
                    pkt_res_devinfo->hdr.version     = DICMOTE_PACKET_VERSION;
                    pkt_res_devinfo->hdr.packet_type = DICMOTE_PACKET_TYPE_RESPONSE_LIST_DEVICES;

                    // Save list start
                    in_buf   = (char*)device_info_list;
                    long off = sizeof(DicPacketResListDevs);

                    while(device_info_list)
                    {
                        memcpy(((char*)pkt_res_devinfo) + off, &device_info_list->this, sizeof(DeviceInfo));
                        device_info_list = device_info_list->next;
                        off += sizeof(DeviceInfo);
                    }

                    device_info_list = (struct DeviceInfoList*)in_buf;
                    FreeDeviceInfoList(device_info_list);

                    write(cli_sock, pkt_res_devinfo, pkt_res_devinfo->hdr.len);
                    free(pkt_res_devinfo);
                    continue;
                case DICMOTE_PACKET_TYPE_RESPONSE_GET_SDHCI_REGISTERS:
                case DICMOTE_PACKET_TYPE_RESPONSE_LIST_DEVICES:
                case DICMOTE_PACKET_TYPE_RESPONSE_SCSI:
                case DICMOTE_PACKET_TYPE_RESPONSE_ATA_CHS:
                case DICMOTE_PACKET_TYPE_RESPONSE_ATA_LBA_28:
                case DICMOTE_PACKET_TYPE_RESPONSE_ATA_LBA_48:
                case DICMOTE_PACKET_TYPE_RESPONSE_SDHCI:
                case DICMOTE_PACKET_TYPE_RESPONSE_GET_DEVTYPE:
                case DICMOTE_PACKET_TYPE_RESPONSE_GET_USB_DATA:
                case DICMOTE_PACKET_TYPE_RESPONSE_GET_FIREWIRE_DATA:
                case DICMOTE_PACKET_TYPE_RESPONSE_GET_PCMCIA_DATA:
                    pkt_nop->reason_code = DICMOTE_PACKET_NOP_REASON_OOO;
                    memset(&pkt_nop->reason, 0, 256);
                    strncpy(pkt_nop->reason, "Received response packet?! You should certainly not do that...", 256);
                    write(cli_sock, pkt_nop, sizeof(DicPacketNop));
                    printf("%s...\n", pkt_nop->reason);
                    skip_next_hdr = 1;
                    continue;
                case DICMOTE_PACKET_TYPE_COMMAND_OPEN_DEVICE:
                    pkt_dev_open = malloc(pkt_hdr->len);

                    if(!pkt_dev_open)
                    {
                        printf("Fatal error %d allocating memory for packet, closing connection...\n", errno);
                        free(pkt_hdr);
                        close(cli_sock);
                        continue;
                    }

                    recv(cli_sock, pkt_dev_open, pkt_hdr->len, 0);

                    device_fd = DeviceOpen(pkt_dev_open->device_path);

                    pkt_nop->reason_code =
                        device_fd == -1 ? DICMOTE_PACKET_NOP_REASON_OPEN_ERROR : DICMOTE_PACKET_NOP_REASON_OPEN_OK;
                    pkt_nop->error_no = errno;
                    memset(&pkt_nop->reason, 0, 256);
                    write(cli_sock, pkt_nop, sizeof(DicPacketNop));

                    if(pkt_nop->reason_code == DICMOTE_PACKET_NOP_REASON_OPEN_OK)
                        strncpy(device_path, pkt_dev_open->device_path, 1024);

                    free(pkt_dev_open);
                    continue;
                case DICMOTE_PACKET_TYPE_COMMAND_GET_DEVTYPE:
                    // Packet only contains header so, dummy
                    in_buf = malloc(pkt_hdr->len);

                    if(!in_buf)
                    {
                        printf("Fatal error %d allocating memory for packet, closing connection...\n", errno);
                        free(pkt_hdr);
                        close(cli_sock);
                        continue;
                    }

                    recv(cli_sock, in_buf, pkt_hdr->len, 0);
                    free(in_buf);

                    pkt_dev_type = malloc(sizeof(DicPacketResGetDeviceType));

                    if(!pkt_dev_type)
                    {
                        printf("Fatal error %d allocating memory for packet, closing connection...\n", errno);
                        free(pkt_hdr);
                        close(cli_sock);
                        continue;
                    }

                    memset(pkt_dev_type, 0, sizeof(DicPacketResGetDeviceType));

                    pkt_dev_type->hdr.len         = sizeof(DicPacketResGetDeviceType);
                    pkt_dev_type->hdr.packet_type = DICMOTE_PACKET_TYPE_RESPONSE_GET_DEVTYPE;
                    pkt_dev_type->hdr.version     = DICMOTE_PACKET_VERSION;
                    pkt_dev_type->hdr.id          = DICMOTE_PACKET_ID;
                    pkt_dev_type->device_type     = GetDeviceType(device_path);

                    write(cli_sock, pkt_dev_type, sizeof(DicPacketResGetDeviceType));
                    free(pkt_dev_type);
                    continue;
                case DICMOTE_PACKET_TYPE_COMMAND_SCSI:
                    // Packet contains data after
                    in_buf = malloc(pkt_hdr->len);

                    if(!in_buf)
                    {
                        printf("Fatal error %d allocating memory for packet, closing connection...\n", errno);
                        free(pkt_hdr);
                        close(cli_sock);
                        continue;
                    }

                    recv(cli_sock, in_buf, pkt_hdr->len, 0);

                    pkt_cmd_scsi = (DicPacketCmdScsi*)in_buf;

                    // TODO: Check size of buffers + size of packet is not bigger than size in header

                    if(pkt_cmd_scsi->cdb_len > 0) cdb_buf = in_buf + sizeof(DicPacketCmdScsi);
                    else
                        cdb_buf = NULL;

                    if(pkt_cmd_scsi->buf_len > 0) buffer = in_buf + pkt_cmd_scsi->cdb_len + sizeof(DicPacketCmdScsi);
                    else
                        buffer = NULL;

                    ret = SendScsiCommand(device_fd,
                                          cdb_buf,
                                          buffer,
                                          &sense_buf,
                                          pkt_cmd_scsi->timeout,
                                          pkt_cmd_scsi->direction,
                                          &duration,
                                          &sense,
                                          pkt_cmd_scsi->cdb_len,
                                          &pkt_cmd_scsi->buf_len,
                                          &sense_len);

                    out_buf = malloc(sizeof(DicPacketResScsi) + sense_len + pkt_cmd_scsi->buf_len);

                    if(!out_buf)
                    {
                        printf("Fatal error %d allocating memory for packet, continuing...\n", errno);
                        free(pkt_hdr);
                        free(in_buf);
                        close(cli_sock);
                        continue;
                    }

                    pkt_res_scsi = (DicPacketResScsi*)out_buf;
                    if(sense_buf) memcpy(out_buf + sizeof(DicPacketResScsi), sense_buf, sense_len);
                    if(buffer) memcpy(out_buf + sizeof(DicPacketResScsi) + sense_len, buffer, pkt_cmd_scsi->buf_len);

                    pkt_res_scsi->hdr.len         = sizeof(DicPacketResScsi) + sense_len + pkt_cmd_scsi->buf_len;
                    pkt_res_scsi->hdr.packet_type = DICMOTE_PACKET_TYPE_RESPONSE_SCSI;
                    pkt_res_scsi->hdr.version     = DICMOTE_PACKET_VERSION;
                    pkt_res_scsi->hdr.id          = DICMOTE_PACKET_ID;

                    pkt_res_scsi->sense_len = sense_len;
                    pkt_res_scsi->buf_len   = pkt_cmd_scsi->buf_len;
                    pkt_res_scsi->duration  = duration;
                    pkt_res_scsi->sense     = sense;
                    pkt_res_scsi->error_no  = ret;

                    write(cli_sock, pkt_res_scsi, pkt_res_scsi->hdr.len);
                    free(pkt_cmd_scsi);
                    free(pkt_res_scsi);
                    if(sense_buf) free(sense_buf);
                    continue;
                case DICMOTE_PACKET_TYPE_COMMAND_GET_SDHCI_REGISTERS:
                    // Packet only contains header so, dummy
                    in_buf = malloc(pkt_hdr->len);

                    if(!in_buf)
                    {
                        printf("Fatal error %d allocating memory for packet, closing connection...\n", errno);
                        free(pkt_hdr);
                        close(cli_sock);
                        continue;
                    }

                    recv(cli_sock, in_buf, pkt_hdr->len, 0);
                    free(in_buf);

                    pkt_res_sdhci_registers = malloc(sizeof(DicPacketResGetSdhciRegisters));
                    if(!pkt_res_sdhci_registers)
                    {
                        printf("Fatal error %d allocating memory for packet, closing connection...\n", errno);
                        free(pkt_hdr);
                        close(cli_sock);
                        continue;
                    }

                    memset(pkt_res_sdhci_registers, 0, sizeof(DicPacketResGetSdhciRegisters));
                    pkt_res_sdhci_registers->hdr.id          = DICMOTE_PACKET_ID;
                    pkt_res_sdhci_registers->hdr.version     = DICMOTE_PACKET_VERSION;
                    pkt_res_sdhci_registers->hdr.packet_type = DICMOTE_PACKET_TYPE_RESPONSE_GET_SDHCI_REGISTERS;
                    pkt_res_sdhci_registers->hdr.len         = sizeof(DicPacketResGetSdhciRegisters);
                    pkt_res_sdhci_registers->is_sdhci        = GetSdhciRegisters(device_path,
                                                                          &csd,
                                                                          &cid,
                                                                          &ocr,
                                                                          &scr,
                                                                          &pkt_res_sdhci_registers->csd_len,
                                                                          &pkt_res_sdhci_registers->cid_len,
                                                                          &pkt_res_sdhci_registers->ocr_len,
                                                                          &pkt_res_sdhci_registers->scr_len);

                    if(pkt_res_sdhci_registers->csd_len > 0 && csd != NULL)
                    {
                        if(pkt_res_sdhci_registers->csd_len > 16) pkt_res_sdhci_registers->csd_len = 16;

                        memcpy(pkt_res_sdhci_registers->csd, csd, pkt_res_sdhci_registers->csd_len);
                    }
                    if(pkt_res_sdhci_registers->cid_len > 0 && cid != NULL)
                    {
                        if(pkt_res_sdhci_registers->cid_len > 16) pkt_res_sdhci_registers->cid_len = 16;

                        memcpy(pkt_res_sdhci_registers->cid, cid, pkt_res_sdhci_registers->cid_len);
                    }
                    if(pkt_res_sdhci_registers->ocr_len > 0 && ocr != NULL)
                    {
                        if(pkt_res_sdhci_registers->ocr_len > 4) pkt_res_sdhci_registers->ocr_len = 4;

                        memcpy(pkt_res_sdhci_registers->ocr, ocr, pkt_res_sdhci_registers->ocr_len);
                    }
                    if(pkt_res_sdhci_registers->scr_len > 0 && scr != NULL)
                    {
                        if(pkt_res_sdhci_registers->scr_len > 8) pkt_res_sdhci_registers->scr_len = 8;

                        memcpy(pkt_res_sdhci_registers->scr, scr, pkt_res_sdhci_registers->scr_len);
                    }

                    free(csd);
                    free(cid);
                    free(scr);
                    free(ocr);

                    write(cli_sock, pkt_res_sdhci_registers, pkt_res_sdhci_registers->hdr.len);
                    free(pkt_res_sdhci_registers);
                    continue;
                case DICMOTE_PACKET_TYPE_COMMAND_GET_USB_DATA:
                    // Packet only contains header so, dummy
                    in_buf = malloc(pkt_hdr->len);

                    if(!in_buf)
                    {
                        printf("Fatal error %d allocating memory for packet, closing connection...\n", errno);
                        free(pkt_hdr);
                        close(cli_sock);
                        continue;
                    }

                    recv(cli_sock, in_buf, pkt_hdr->len, 0);
                    free(in_buf);

                    pkt_res_usb = malloc(sizeof(DicPacketResGetUsbData));
                    if(!pkt_res_usb)
                    {
                        printf("Fatal error %d allocating memory for packet, closing connection...\n", errno);
                        free(pkt_hdr);
                        close(cli_sock);
                        continue;
                    }

                    memset(pkt_res_usb, 0, sizeof(DicPacketResGetUsbData));
                    pkt_res_usb->hdr.id          = DICMOTE_PACKET_ID;
                    pkt_res_usb->hdr.version     = DICMOTE_PACKET_VERSION;
                    pkt_res_usb->hdr.packet_type = DICMOTE_PACKET_TYPE_RESPONSE_GET_USB_DATA;
                    pkt_res_usb->hdr.len         = sizeof(DicPacketResGetUsbData);
                    pkt_res_usb->is_usb          = GetUsbData(device_path,
                                                     &pkt_res_usb->desc_len,
                                                     pkt_res_usb->descriptors,
                                                     &pkt_res_usb->id_vendor,
                                                     &pkt_res_usb->id_product,
                                                     pkt_res_usb->manufacturer,
                                                     pkt_res_usb->product,
                                                     pkt_res_usb->serial);

                    write(cli_sock, pkt_res_usb, pkt_res_usb->hdr.len);
                    free(pkt_res_usb);
                    continue;
                case DICMOTE_PACKET_TYPE_COMMAND_GET_FIREWIRE_DATA:
                    // Packet only contains header so, dummy
                    in_buf = malloc(pkt_hdr->len);

                    if(!in_buf)
                    {
                        printf("Fatal error %d allocating memory for packet, closing connection...\n", errno);
                        free(pkt_hdr);
                        close(cli_sock);
                        continue;
                    }

                    recv(cli_sock, in_buf, pkt_hdr->len, 0);
                    free(in_buf);

                    pkt_res_firewire = malloc(sizeof(DicPacketResGetFireWireData));
                    if(!pkt_res_firewire)
                    {
                        printf("Fatal error %d allocating memory for packet, closing connection...\n", errno);
                        free(pkt_hdr);
                        close(cli_sock);
                        continue;
                    }

                    memset(pkt_res_firewire, 0, sizeof(DicPacketResGetFireWireData));
                    pkt_res_firewire->hdr.id          = DICMOTE_PACKET_ID;
                    pkt_res_firewire->hdr.version     = DICMOTE_PACKET_VERSION;
                    pkt_res_firewire->hdr.packet_type = DICMOTE_PACKET_TYPE_RESPONSE_GET_FIREWIRE_DATA;
                    pkt_res_firewire->hdr.len         = sizeof(DicPacketResGetFireWireData);
                    pkt_res_firewire->is_firewire     = GetFireWireData(device_path,
                                                                    &pkt_res_firewire->id_model,
                                                                    &pkt_res_firewire->id_vendor,
                                                                    &pkt_res_firewire->guid,
                                                                    pkt_res_firewire->vendor,
                                                                    pkt_res_firewire->model);

                    write(cli_sock, pkt_res_firewire, pkt_res_firewire->hdr.len);
                    free(pkt_res_firewire);
                    continue;
                case DICMOTE_PACKET_TYPE_COMMAND_GET_PCMCIA_DATA:
                    // Packet only contains header so, dummy
                    in_buf = malloc(pkt_hdr->len);

                    if(!in_buf)
                    {
                        printf("Fatal error %d allocating memory for packet, closing connection...\n", errno);
                        free(pkt_hdr);
                        close(cli_sock);
                        continue;
                    }

                    recv(cli_sock, in_buf, pkt_hdr->len, 0);
                    free(in_buf);

                    pkt_res_pcmcia = malloc(sizeof(DicPacketResGetPcmciaData));
                    if(!pkt_res_pcmcia)
                    {
                        printf("Fatal error %d allocating memory for packet, closing connection...\n", errno);
                        free(pkt_hdr);
                        close(cli_sock);
                        continue;
                    }

                    memset(pkt_res_pcmcia, 0, sizeof(DicPacketResGetPcmciaData));
                    pkt_res_pcmcia->hdr.id          = DICMOTE_PACKET_ID;
                    pkt_res_pcmcia->hdr.version     = DICMOTE_PACKET_VERSION;
                    pkt_res_pcmcia->hdr.packet_type = DICMOTE_PACKET_TYPE_RESPONSE_GET_PCMCIA_DATA;
                    pkt_res_pcmcia->hdr.len         = sizeof(DicPacketResGetPcmciaData);
                    pkt_res_pcmcia->is_pcmcia =
                        GetPcmciaData(device_path, &pkt_res_pcmcia->cis_len, pkt_res_pcmcia->cis);

                    write(cli_sock, pkt_res_pcmcia, pkt_res_pcmcia->hdr.len);
                    free(pkt_res_pcmcia);
                    continue;
                case DICMOTE_PACKET_TYPE_COMMAND_ATA_CHS:
                    // Packet contains data after
                    in_buf = malloc(pkt_hdr->len);

                    if(!in_buf)
                    {
                        printf("Fatal error %d allocating memory for packet, closing connection...\n", errno);
                        free(pkt_hdr);
                        close(cli_sock);
                        continue;
                    }

                    recv(cli_sock, in_buf, pkt_hdr->len, 0);

                    pkt_cmd_ata_chs = (DicPacketCmdAtaChs*)in_buf;

                    // TODO: Check size of buffers + size of packet is not bigger than size in header

                    if(pkt_cmd_ata_chs->buf_len > 0) buffer = in_buf + sizeof(DicPacketCmdAtaChs);
                    else
                        buffer = NULL;

                    memset(&ata_chs_error_regs, 0, sizeof(AtaErrorRegistersChs));

                    duration = 0;
                    sense    = 1;
                    ret      = SendAtaChsCommand(device_fd,
                                            pkt_cmd_ata_chs->registers,
                                            &ata_chs_error_regs,
                                            pkt_cmd_ata_chs->protocol,
                                            pkt_cmd_ata_chs->transfer_register,
                                            buffer,
                                            pkt_cmd_ata_chs->timeout,
                                            pkt_cmd_ata_chs->transfer_blocks,
                                            &duration,
                                            &sense,
                                            &pkt_cmd_ata_chs->buf_len);

                    out_buf = malloc(sizeof(DicPacketResAtaChs) + pkt_cmd_ata_chs->buf_len);

                    if(!out_buf)
                    {
                        printf("Fatal error %d allocating memory for packet, continuing...\n", errno);
                        free(pkt_hdr);
                        free(in_buf);
                        close(cli_sock);
                        continue;
                    }

                    pkt_res_ata_chs = (DicPacketResAtaChs*)out_buf;
                    if(buffer) memcpy(out_buf + sizeof(DicPacketResAtaChs), buffer, pkt_cmd_ata_chs->buf_len);

                    pkt_res_ata_chs->hdr.len         = sizeof(DicPacketResAtaChs) + pkt_cmd_ata_chs->buf_len;
                    pkt_res_ata_chs->hdr.packet_type = DICMOTE_PACKET_TYPE_RESPONSE_ATA_CHS;
                    pkt_res_ata_chs->hdr.version     = DICMOTE_PACKET_VERSION;
                    pkt_res_ata_chs->hdr.id          = DICMOTE_PACKET_ID;

                    pkt_res_ata_chs->registers = ata_chs_error_regs;
                    pkt_res_ata_chs->buf_len   = pkt_cmd_ata_chs->buf_len;
                    pkt_res_ata_chs->duration  = duration;
                    pkt_res_ata_chs->sense     = sense;
                    pkt_res_ata_chs->error_no  = ret;

                    write(cli_sock, pkt_res_ata_chs, pkt_res_ata_chs->hdr.len);
                    free(pkt_cmd_ata_chs);
                    free(pkt_res_ata_chs);
                    continue;
                case DICMOTE_PACKET_TYPE_COMMAND_ATA_LBA_28:
                    // Packet contains data after
                    in_buf = malloc(pkt_hdr->len);

                    if(!in_buf)
                    {
                        printf("Fatal error %d allocating memory for packet, closing connection...\n", errno);
                        free(pkt_hdr);
                        close(cli_sock);
                        continue;
                    }

                    recv(cli_sock, in_buf, pkt_hdr->len, 0);

                    pkt_cmd_ata_lba28 = (DicPacketCmdAtaLba28*)in_buf;

                    // TODO: Check size of buffers + size of packet is not bigger than size in header

                    if(pkt_cmd_ata_lba28->buf_len > 0) buffer = in_buf + sizeof(DicPacketCmdAtaLba28);
                    else
                        buffer = NULL;

                    memset(&ata_lba28_error_regs, 0, sizeof(AtaErrorRegistersLba28));

                    duration = 0;
                    sense    = 1;
                    ret      = SendAtaLba28Command(device_fd,
                                              pkt_cmd_ata_lba28->registers,
                                              &ata_lba28_error_regs,
                                              pkt_cmd_ata_lba28->protocol,
                                              pkt_cmd_ata_lba28->transfer_register,
                                              buffer,
                                              pkt_cmd_ata_lba28->timeout,
                                              pkt_cmd_ata_lba28->transfer_blocks,
                                              &duration,
                                              &sense,
                                              &pkt_cmd_ata_lba28->buf_len);

                    out_buf = malloc(sizeof(DicPacketResAtaLba28) + pkt_cmd_ata_lba28->buf_len);

                    if(!out_buf)
                    {
                        printf("Fatal error %d allocating memory for packet, continuing...\n", errno);
                        free(pkt_hdr);
                        free(in_buf);
                        close(cli_sock);
                        continue;
                    }

                    pkt_res_ata_lba28 = (DicPacketResAtaLba28*)out_buf;
                    if(buffer) memcpy(out_buf + sizeof(DicPacketResAtaLba28), buffer, pkt_cmd_ata_lba28->buf_len);

                    pkt_res_ata_lba28->hdr.len         = sizeof(DicPacketResAtaLba28) + pkt_cmd_ata_lba28->buf_len;
                    pkt_res_ata_lba28->hdr.packet_type = DICMOTE_PACKET_TYPE_RESPONSE_ATA_LBA_28;
                    pkt_res_ata_lba28->hdr.version     = DICMOTE_PACKET_VERSION;
                    pkt_res_ata_lba28->hdr.id          = DICMOTE_PACKET_ID;

                    pkt_res_ata_lba28->registers = ata_lba28_error_regs;
                    pkt_res_ata_lba28->buf_len   = pkt_cmd_ata_lba28->buf_len;
                    pkt_res_ata_lba28->duration  = duration;
                    pkt_res_ata_lba28->sense     = sense;
                    pkt_res_ata_lba28->error_no  = ret;

                    write(cli_sock, pkt_res_ata_lba28, pkt_res_ata_lba28->hdr.len);
                    free(pkt_cmd_ata_lba28);
                    free(pkt_res_ata_lba28);
                    continue;
                case DICMOTE_PACKET_TYPE_COMMAND_ATA_LBA_48:
                    // Packet contains data after
                    in_buf = malloc(pkt_hdr->len);

                    if(!in_buf)
                    {
                        printf("Fatal error %d allocating memory for packet, closing connection...\n", errno);
                        free(pkt_hdr);
                        close(cli_sock);
                        continue;
                    }

                    recv(cli_sock, in_buf, pkt_hdr->len, 0);

                    pkt_cmd_ata_lba48 = (DicPacketCmdAtaLba48*)in_buf;

                    // TODO: Check size of buffers + size of packet is not bigger than size in header

                    if(pkt_cmd_ata_lba48->buf_len > 0) buffer = in_buf + sizeof(DicPacketCmdAtaLba48);
                    else
                        buffer = NULL;

                    memset(&ata_lba48_error_regs, 0, sizeof(AtaErrorRegistersLba48));

                    duration = 0;
                    sense    = 1;
                    ret      = SendAtaLba48Command(device_fd,
                                              pkt_cmd_ata_lba48->registers,
                                              &ata_lba48_error_regs,
                                              pkt_cmd_ata_lba48->protocol,
                                              pkt_cmd_ata_lba48->transfer_register,
                                              buffer,
                                              pkt_cmd_ata_lba48->timeout,
                                              pkt_cmd_ata_lba48->transfer_blocks,
                                              &duration,
                                              &sense,
                                              &pkt_cmd_ata_lba48->buf_len);

                    out_buf = malloc(sizeof(DicPacketResAtaLba48) + pkt_cmd_ata_lba48->buf_len);

                    if(!out_buf)
                    {
                        printf("Fatal error %d allocating memory for packet, continuing...\n", errno);
                        free(pkt_hdr);
                        free(in_buf);
                        close(cli_sock);
                        continue;
                    }

                    pkt_res_ata_lba48 = (DicPacketResAtaLba48*)out_buf;
                    if(buffer) memcpy(out_buf + sizeof(DicPacketResAtaLba48), buffer, pkt_cmd_ata_lba48->buf_len);

                    pkt_res_ata_lba48->hdr.len         = sizeof(DicPacketResAtaLba48) + pkt_cmd_ata_lba48->buf_len;
                    pkt_res_ata_lba48->hdr.packet_type = DICMOTE_PACKET_TYPE_RESPONSE_ATA_LBA_48;
                    pkt_res_ata_lba48->hdr.version     = DICMOTE_PACKET_VERSION;
                    pkt_res_ata_lba48->hdr.id          = DICMOTE_PACKET_ID;

                    pkt_res_ata_lba48->registers = ata_lba48_error_regs;
                    pkt_res_ata_lba48->buf_len   = pkt_cmd_ata_lba48->buf_len;
                    pkt_res_ata_lba48->duration  = duration;
                    pkt_res_ata_lba48->sense     = sense;
                    pkt_res_ata_lba48->error_no  = ret;

                    write(cli_sock, pkt_res_ata_lba48, pkt_res_ata_lba48->hdr.len);
                    free(pkt_cmd_ata_lba48);
                    free(pkt_res_ata_lba48);
                    continue;
                case DICMOTE_PACKET_TYPE_COMMAND_SDHCI:
                    // Packet contains data after
                    in_buf = malloc(pkt_hdr->len);

                    if(!in_buf)
                    {
                        printf("Fatal error %d allocating memory for packet, closing connection...\n", errno);
                        free(pkt_hdr);
                        close(cli_sock);
                        continue;
                    }

                    recv(cli_sock, in_buf, pkt_hdr->len, 0);

                    pkt_cmd_sdhci = (DicPacketCmdSdhci*)in_buf;

                    // TODO: Check size of buffers + size of packet is not bigger than size in header

                    if(pkt_cmd_sdhci->buf_len > 0) buffer = in_buf + sizeof(DicPacketCmdSdhci);
                    else
                        buffer = NULL;

                    memset((char*)&sdhci_response, 0, sizeof(uint32_t) * 4);

                    duration = 0;
                    sense    = 1;
                    ret      = SendSdhciCommand(device_fd,
                                           pkt_cmd_sdhci->command,
                                           pkt_cmd_sdhci->write,
                                           pkt_cmd_sdhci->application,
                                           pkt_cmd_sdhci->flags,
                                           pkt_cmd_sdhci->argument,
                                           pkt_cmd_sdhci->block_size,
                                           pkt_cmd_sdhci->blocks,
                                           buffer,
                                           pkt_cmd_sdhci->buf_len,
                                           pkt_cmd_sdhci->timeout,
                                           (uint32_t*)&sdhci_response,
                                           &duration,
                                           &sense);

                    out_buf = malloc(sizeof(DicPacketResSdhci) + pkt_cmd_sdhci->buf_len);

                    if(!out_buf)
                    {
                        printf("Fatal error %d allocating memory for packet, continuing...\n", errno);
                        free(pkt_hdr);
                        free(in_buf);
                        close(cli_sock);
                        continue;
                    }

                    pkt_res_sdhci = (DicPacketResSdhci*)out_buf;
                    if(buffer) memcpy(out_buf + sizeof(DicPacketResSdhci), buffer, pkt_cmd_sdhci->buf_len);

                    pkt_res_sdhci->hdr.len         = sizeof(DicPacketResSdhci) + pkt_cmd_sdhci->buf_len;
                    pkt_res_sdhci->hdr.packet_type = DICMOTE_PACKET_TYPE_RESPONSE_SDHCI;
                    pkt_res_sdhci->hdr.version     = DICMOTE_PACKET_VERSION;
                    pkt_res_sdhci->hdr.id          = DICMOTE_PACKET_ID;

                    memcpy((char*)&pkt_res_sdhci->response, (char*)&sdhci_response, sizeof(uint32_t) * 4);
                    pkt_res_sdhci->buf_len  = pkt_cmd_sdhci->buf_len;
                    pkt_res_sdhci->duration = duration;
                    pkt_res_sdhci->sense    = sense;
                    pkt_res_sdhci->error_no = ret;

                    write(cli_sock, pkt_res_sdhci, pkt_res_sdhci->hdr.len);
                    free(pkt_cmd_sdhci);
                    free(pkt_res_sdhci);
                    continue;
                default:
                    pkt_nop->reason_code = DICMOTE_PACKET_NOP_REASON_NOT_RECOGNIZED;
                    memset(&pkt_nop->reason, 0, 256);
                    snprintf(pkt_nop->reason,
                             256,
                             "Received unrecognized packet with type %d, skipping...",
                             pkt_hdr->packet_type);
                    write(cli_sock, pkt_nop, sizeof(DicPacketNop));
                    printf("%s...\n", pkt_nop->reason);
                    skip_next_hdr = 1;
                    continue;
            }
        }
    }
}