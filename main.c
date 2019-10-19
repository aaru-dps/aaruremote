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
#include <ifaddrs.h>
#include <libnet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/utsname.h>

// TODO: Packet for NOP

int main()
{
    struct ifaddrs*                ifa;
    struct ifaddrs*                ifa_start;
    int                            ret;
    char                           ipv4Address[INET_ADDRSTRLEN];
    int                            sockfd, cli_sock;
    struct sockaddr_in             serv_addr, cli_addr;
    socklen_t                      cli_len;
    struct utsname                 utsname;
    DicPacketHello *               pkt_server_hello, *pkt_client_hello;
    DicPacketHeader*               pkt_hdr;
    ssize_t                        recv_size;
    char*                          in_buf;
    int                            skip_next_hdr;
    struct DeviceInfoList*         deviceInfoList;
    DicPacketResListDevs*          deviceInfoResponsePacket;
    int                            i;
    uint64_t                       n;
    DicPacketNop*                  pkt_nop;
    DicPacketCmdOpen*              pkt_dev_open;
    int                            device_fd = -1;
    char                           device_path[1024];
    DicPacketResGetDeviceType*     pkt_dev_type;
    DicPacketCmdScsi*              pkt_cmd_scsi;
    char*                          sense_buf;
    char*                          buffer;
    char*                          cdb_buf;
    uint32_t                       duration;
    uint32_t                       sense;
    uint32_t                       sense_len;
    char*                          out_buf;
    DicPacketResScsi*              pkt_res_scsi;
    DicPacketResGetSdhciRegisters* sdhciRegsResponsePacket;
    char*                          csd;
    char*                          cid;
    char*                          ocr;
    char*                          scr;
    DicPacketResGetUsbData*        pkt_res_usb;
    DicPacketResGetFireWireData*   pkt_res_firewire;
    DicPacketResGetPcmciaData*     pkt_res_pcmcia;

    printf("DiscImageChef Remote Server %s\n", DICMOTE_VERSION);
    printf("Copyright (C) 2019 Natalia Portillo\n");

    ret = uname(&utsname);

    if(ret)
    {
        printf("Error %d getting system version.\n", errno);
        return 1;
    }

    printf("Running under %s %s (%s).\n", utsname.sysname, utsname.release, utsname.machine);

    printf("Opening socket.\n");
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
    {
        printf("Error %d opening socket.\n", errno);
        return 1;
    }

    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port        = htons(DICMOTE_PORT);

    if(bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("Error %d binding socket.\n", errno);
        close(sockfd);
        return 1;
    }

    ret = getifaddrs(&ifa);

    if(ret)
    {
        printf("Error %d enumerating interfaces\n", errno);
        return 1;
    }

    ifa_start = ifa;

    printf("Available addresses:\n");
    while(ifa != NULL)
    {
        if(ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET)
        {
            inet_ntop(AF_INET, &((struct sockaddr_in*)ifa->ifa_addr)->sin_addr, ipv4Address, INET_ADDRSTRLEN);
            printf("%s port %d\n", ipv4Address, DICMOTE_PORT);
        }

        ifa = ifa->ifa_next;
    }

    freeifaddrs(ifa_start);

    ret = listen(sockfd, 1);

    if(ret)
    {
        printf("Error %d listening.\n", errno);
        close(sockfd);
        return 1;
    }

    pkt_server_hello = malloc(sizeof(DicPacketHello));

    if(!pkt_server_hello)
    {
        printf("Fatal error %d allocating memory.\n", errno);
        close(sockfd);
        return 1;
    }

    memset(pkt_server_hello, 0, sizeof(DicPacketHello));

    pkt_server_hello->hdr.id          = DICMOTE_PACKET_ID;
    pkt_server_hello->hdr.len         = sizeof(DicPacketHello);
    pkt_server_hello->hdr.version     = DICMOTE_PACKET_VERSION;
    pkt_server_hello->hdr.packet_type = DICMOTE_PACKET_TYPE_HELLO;
    strncpy(pkt_server_hello->application, DICMOTE_NAME, sizeof(DICMOTE_NAME));
    strncpy(pkt_server_hello->version, DICMOTE_VERSION, sizeof(DICMOTE_VERSION));
    pkt_server_hello->max_protocol = DICMOTE_PROTOCOL_MAX;
    strncpy(pkt_server_hello->sysname, utsname.sysname, 255);
    strncpy(pkt_server_hello->release, utsname.release, 255);
    strncpy(pkt_server_hello->machine, utsname.machine, 255);

    pkt_nop = malloc(sizeof(DicPacketNop));

    if(!pkt_nop)
    {
        printf("Fatal error %d allocating memory.\n", errno);
        close(sockfd);
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
        cli_sock = accept(sockfd, (struct sockaddr*)&cli_addr, &cli_len);

        if(cli_sock < 0)
        {
            printf("Error %d accepting incoming connection.\n", errno);
            close(sockfd);
            return 1;
        }

        inet_ntop(AF_INET, &cli_addr.sin_addr, ipv4Address, INET_ADDRSTRLEN);
        printf("Client %s connected successfully.\n", ipv4Address);

        write(cli_sock, pkt_server_hello, sizeof(DicPacketHello));

        pkt_hdr = malloc(sizeof(DicPacketHeader));

        if(!pkt_hdr)
        {
            printf("Fatal error %d allocating memory.\n", errno);
            close(cli_sock);
            close(sockfd);
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
                    deviceInfoList = ListDevices();

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

                    if(!deviceInfoList)
                    {
                        pkt_nop->reason_code = DICMOTE_PACKET_NOP_REASON_ERROR_LIST_DEVICES;
                        memset(&pkt_nop->reason, 0, 256);
                        strncpy(pkt_nop->reason, "Could not get device list, continuing...", 256);
                        write(cli_sock, pkt_nop, sizeof(DicPacketNop));
                        printf("%s...\n", pkt_nop->reason);
                        continue;
                    }

                    deviceInfoResponsePacket          = malloc(sizeof(DicPacketResListDevs));
                    deviceInfoResponsePacket->devices = DeviceInfoListCount(deviceInfoList);

                    n      = sizeof(DicPacketResListDevs) + deviceInfoResponsePacket->devices * sizeof(DeviceInfo);
                    in_buf = malloc(n);
                    ((DicPacketResListDevs*)in_buf)->hdr.len = n;
                    ((DicPacketResListDevs*)in_buf)->devices = deviceInfoResponsePacket->devices;
                    free(deviceInfoResponsePacket);
                    deviceInfoResponsePacket = (DicPacketResListDevs*)in_buf;
                    in_buf                   = NULL;

                    deviceInfoResponsePacket->hdr.id          = DICMOTE_PACKET_ID;
                    deviceInfoResponsePacket->hdr.version     = DICMOTE_PACKET_VERSION;
                    deviceInfoResponsePacket->hdr.packet_type = DICMOTE_PACKET_TYPE_RESPONSE_LIST_DEVICES;

                    // Save list start
                    in_buf   = (char*)deviceInfoList;
                    long off = sizeof(DicPacketResListDevs);

                    while(deviceInfoList)
                    {
                        memcpy(((char*)deviceInfoResponsePacket) + off, &deviceInfoList->this, sizeof(DeviceInfo));
                        deviceInfoList = deviceInfoList->next;
                        off += sizeof(DeviceInfo);
                    }

                    deviceInfoList = (struct DeviceInfoList*)in_buf;
                    FreeDeviceInfoList(deviceInfoList);

                    write(cli_sock, deviceInfoResponsePacket, deviceInfoResponsePacket->hdr.len);
                    free(deviceInfoResponsePacket);
                    continue;
                case DICMOTE_PACKET_TYPE_RESPONSE_GET_SDHCI_REGISTERS:
                case DICMOTE_PACKET_TYPE_RESPONSE_LIST_DEVICES:
                case DICMOTE_PACKET_TYPE_RESPONSE_SCSI:
                case DICMOTE_PACKET_TYPE_RESPONSE_ATA_CHS:
                case DICMOTE_PACKET_TYPE_RESPONSE_ATA_LBA28:
                case DICMOTE_PACKET_TYPE_RESPONSE_ATA_LBA48:
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
                    pkt_nop->errorNo = errno;
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

                    if(pkt_cmd_scsi->cdb_len > 0)
                        cdb_buf = in_buf + sizeof(DicPacketCmdScsi);
                    else
                        cdb_buf = NULL;

                    if(pkt_cmd_scsi->buf_len > 0)
                        buffer = in_buf + pkt_cmd_scsi->cdb_len + sizeof(DicPacketCmdScsi);
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

                    sdhciRegsResponsePacket = malloc(sizeof(DicPacketResGetSdhciRegisters));
                    if(!sdhciRegsResponsePacket)
                    {
                        printf("Fatal error %d allocating memory for packet, closing connection...\n", errno);
                        free(pkt_hdr);
                        close(cli_sock);
                        continue;
                    }

                    memset(sdhciRegsResponsePacket, 0, sizeof(DicPacketResGetSdhciRegisters));
                    sdhciRegsResponsePacket->hdr.id          = DICMOTE_PACKET_ID;
                    sdhciRegsResponsePacket->hdr.version     = DICMOTE_PACKET_VERSION;
                    sdhciRegsResponsePacket->hdr.packet_type = DICMOTE_PACKET_TYPE_RESPONSE_GET_SDHCI_REGISTERS;
                    sdhciRegsResponsePacket->hdr.len         = sizeof(DicPacketResGetSdhciRegisters);
                    sdhciRegsResponsePacket->isSdhci         = GetSdhciRegisters(device_path,
                                                                         &csd,
                                                                         &cid,
                                                                         &ocr,
                                                                         &scr,
                                                                         &sdhciRegsResponsePacket->csd_len,
                                                                         &sdhciRegsResponsePacket->cid_len,
                                                                         &sdhciRegsResponsePacket->ocr_len,
                                                                         &sdhciRegsResponsePacket->scr_len);

                    if(sdhciRegsResponsePacket->csd_len > 0 && csd != NULL)
                    {
                        if(sdhciRegsResponsePacket->csd_len > 16) sdhciRegsResponsePacket->csd_len = 16;

                        memcpy(sdhciRegsResponsePacket->csd, csd, sdhciRegsResponsePacket->csd_len);
                    }
                    if(sdhciRegsResponsePacket->cid_len > 0 && cid != NULL)
                    {
                        if(sdhciRegsResponsePacket->cid_len > 16) sdhciRegsResponsePacket->cid_len = 16;

                        memcpy(sdhciRegsResponsePacket->cid, cid, sdhciRegsResponsePacket->cid_len);
                    }
                    if(sdhciRegsResponsePacket->ocr_len > 0 && ocr != NULL)
                    {
                        if(sdhciRegsResponsePacket->ocr_len > 4) sdhciRegsResponsePacket->ocr_len = 4;

                        memcpy(sdhciRegsResponsePacket->ocr, ocr, sdhciRegsResponsePacket->ocr_len);
                    }
                    if(sdhciRegsResponsePacket->scr_len > 0 && scr != NULL)
                    {
                        if(sdhciRegsResponsePacket->scr_len > 8) sdhciRegsResponsePacket->scr_len = 8;

                        memcpy(sdhciRegsResponsePacket->scr, scr, sdhciRegsResponsePacket->scr_len);
                    }

                    free(csd);
                    free(cid);
                    free(scr);
                    free(ocr);

                    write(cli_sock, sdhciRegsResponsePacket, sdhciRegsResponsePacket->hdr.len);
                    free(sdhciRegsResponsePacket);
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
                    pkt_res_usb->isUsb           = GetUsbData(device_path,
                                                    &pkt_res_usb->descLen,
                                                    pkt_res_usb->descriptors,
                                                    &pkt_res_usb->idVendor,
                                                    &pkt_res_usb->idProduct,
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
                    pkt_res_firewire->isFireWire      = GetFireWireData(device_path,
                                                                   &pkt_res_firewire->idModel,
                                                                   &pkt_res_firewire->idVendor,
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
                    pkt_res_pcmcia->isPcmcia =
                        GetPcmciaData(device_path, &pkt_res_pcmcia->cis_len, pkt_res_pcmcia->cis);

                    write(cli_sock, pkt_res_pcmcia, pkt_res_pcmcia->hdr.len);
                    free(pkt_res_pcmcia);
                    continue;
                case DICMOTE_PACKET_TYPE_COMMAND_ATA_CHS:
                case DICMOTE_PACKET_TYPE_COMMAND_ATA_LBA28:
                case DICMOTE_PACKET_TYPE_COMMAND_ATA_LBA48:
                case DICMOTE_PACKET_TYPE_COMMAND_SDHCI:
                    pkt_nop->reason_code = DICMOTE_PACKET_NOP_REASON_NOT_IMPLEMENTED;
                    memset(&pkt_nop->reason, 0, 256);
                    strncpy(pkt_nop->reason, "Packet not yet implemented, skipping...", 256);
                    write(cli_sock, pkt_nop, sizeof(DicPacketNop));
                    printf("%s...\n", pkt_nop->reason);
                    skip_next_hdr = 1;
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