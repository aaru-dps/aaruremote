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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define ssize_t int
#include <winsock2.h>
#include <windows.h>

#include "win32/win32.h"
#else
#include <stdint.h>
#endif

#include "aaruremote.h"
#include "endian.h"

void* WorkingLoop(void* arguments)
{
    AtaErrorRegistersChs            ata_chs_error_regs;
    AtaErrorRegistersLba28          ata_lba28_error_regs;
    AtaErrorRegistersLba48          ata_lba48_error_regs;
    char*                           buffer;
    char*                           cdb_buf;
    char*                           cid;
    char*                           csd;
    char*                           in_buf;
    char*                           ocr;
    char*                           out_buf;
    char*                           scr;
    char*                           sense_buf;
    AaruPacketCmdAtaChs*            pkt_cmd_ata_chs;
    AaruPacketCmdAtaLba28*          pkt_cmd_ata_lba28;
    AaruPacketCmdAtaLba48*          pkt_cmd_ata_lba48;
    AaruPacketCmdOpen*              pkt_dev_open;
    AaruPacketCmdScsi*              pkt_cmd_scsi;
    AaruPacketCmdSdhci*             pkt_cmd_sdhci;
    AaruPacketHeader*               pkt_hdr;
    AaruPacketHello*                pkt_server_hello;
    AaruPacketHello*                pkt_client_hello;
    AaruPacketNop*                  pkt_nop;
    AaruPacketResAmIRoot*           pkt_res_am_i_root;
    AaruPacketResAtaChs*            pkt_res_ata_chs;
    AaruPacketResAtaLba28*          pkt_res_ata_lba28;
    AaruPacketResAtaLba48*          pkt_res_ata_lba48;
    AaruPacketResGetDeviceType*     pkt_dev_type;
    AaruPacketResGetFireWireData*   pkt_res_firewire;
    AaruPacketResGetPcmciaData*     pkt_res_pcmcia;
    AaruPacketResGetSdhciRegisters* pkt_res_sdhci_registers;
    AaruPacketResGetUsbData*        pkt_res_usb;
    AaruPacketResListDevs*          pkt_res_devinfo;
    AaruPacketResScsi*              pkt_res_scsi;
    AaruPacketResSdhci*             pkt_res_sdhci;
    int                             skip_next_hdr;
    int                             ret;
    socklen_t                       cli_len;
    ssize_t                         recv_size;
    struct DeviceInfoList*          device_info_list;
    struct sockaddr_in              cli_addr, serv_addr;
    uint32_t                        duration;
    uint32_t                        sdhci_response[4];
    uint32_t                        sense;
    uint32_t                        sense_len;
    uint32_t                        n;
    void*                           device_ctx = NULL;
    void*                           net_ctx    = NULL;
    void*                           cli_ctx    = NULL;
    long                            off;

    if(!arguments)
    {
        printf("Hello packet not sent, returning");
        return NULL;
    }

    pkt_server_hello = (AaruPacketHello*)arguments;

    printf("Opening socket.\n");
    net_ctx = NetSocket(AF_INET, SOCK_STREAM, 0);
    if(!net_ctx)
    {
        printf("Error %d opening socket.\n", errno);
        return NULL;
    }

    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port        = htons(AARUREMOTE_PORT);

    if(NetBind(net_ctx, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("Error %d binding socket.\n", errno);
        NetClose(net_ctx);
        return NULL;
    }

    ret = NetListen(net_ctx, 1);

    if(ret)
    {
        printf("Error %d listening.\n", errno);
        NetClose(net_ctx);
        return NULL;
    }

    pkt_nop = malloc(sizeof(AaruPacketNop));

    if(!pkt_nop)
    {
        printf("Fatal error %d allocating memory.\n", errno);
        NetClose(net_ctx);
        return NULL;
    }

    memset(pkt_nop, 0, sizeof(AaruPacketNop));

    pkt_nop->hdr.remote_id   = htole32(AARUREMOTE_REMOTE_ID);
    pkt_nop->hdr.packet_id   = htole32(AARUREMOTE_PACKET_ID);
    pkt_nop->hdr.len         = htole32(sizeof(AaruPacketNop));
    pkt_nop->hdr.version     = AARUREMOTE_PACKET_VERSION;
    pkt_nop->hdr.packet_type = AARUREMOTE_PACKET_TYPE_NOP;

    for(;;)
    {
        printf("\n");
        printf("Waiting for a client...\n");

        cli_len = sizeof(cli_addr);
        cli_ctx = NetAccept(net_ctx, (struct sockaddr*)&cli_addr, &cli_len);

        if(!cli_ctx)
        {
            printf("Error %d accepting incoming connection.\n", errno);
            free(pkt_nop);
            NetClose(net_ctx);
            return NULL;
        }

        printf("Client %s connected successfully.\n", PrintIpv4Address(cli_addr.sin_addr));

        NetWrite(cli_ctx, pkt_server_hello, sizeof(AaruPacketHello));

        pkt_hdr = malloc(sizeof(AaruPacketHeader));

        if(!pkt_hdr)
        {
            printf("Fatal error %d allocating memory.\n", errno);
            NetClose(cli_ctx);
            NetClose(net_ctx);
            free(pkt_server_hello);
            free(pkt_nop);
            return NULL;
        }

        recv_size = NetRecv(cli_ctx, pkt_hdr, sizeof(AaruPacketHeader), MSG_PEEK);

        if(recv_size < 0)
        {
            printf("Error %d reading response from client.\n", errno);
            free(pkt_hdr);
            NetClose(cli_ctx);
            continue;
        }

        if(recv_size == 0)
        {
            printf("Client closed connection.\n");
            free(pkt_hdr);
            NetClose(cli_ctx);
            continue;
        }

        if(pkt_hdr->remote_id != htole32(AARUREMOTE_REMOTE_ID) || pkt_hdr->packet_id != htole32(AARUREMOTE_PACKET_ID))
        {
            printf("Received data is not a correct aaruremote packet, closing connection...\n");
            free(pkt_hdr);
            NetClose(cli_ctx);
            continue;
        }

        if(pkt_hdr->version != AARUREMOTE_PACKET_VERSION)
        {
            printf("Unrecognized packet version, closing connection...\n");
            free(pkt_hdr);
            NetClose(cli_ctx);
            continue;
        }

        if(pkt_hdr->packet_type != AARUREMOTE_PACKET_TYPE_HELLO)
        {
            printf("Expecting hello packet type, received type %d, closing connection...\n", pkt_hdr->packet_type);
            free(pkt_hdr);
            NetClose(cli_ctx);
            continue;
        }

        pkt_client_hello = malloc(le32toh(pkt_hdr->len));

        if(!pkt_client_hello)
        {
            printf("Fatal error %d allocating memory for packet, closing connection...\n", errno);
            free(pkt_hdr);
            NetClose(cli_ctx);
            continue;
        }

        recv_size = NetRecv(cli_ctx, pkt_client_hello, le32toh(pkt_hdr->len), 0);

        if(recv_size != le32toh(pkt_hdr->len))
        {
            printf("Expected %d bytes of packet, got %ld, closing connection...\n", le32toh(pkt_hdr->len), recv_size);
            NetClose(cli_ctx);
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
                in_buf = malloc(le32toh(pkt_hdr->len));

                if(!in_buf)
                {
                    printf("Fatal error %d allocating memory for packet, closing connection...\n", errno);
                    free(pkt_hdr);
                    NetClose(cli_ctx);
                    continue;
                }

                NetRecv(cli_ctx, in_buf, le32toh(pkt_hdr->len), 0);
                free(in_buf);
                skip_next_hdr = 0;
            }

            recv_size = NetRecv(cli_ctx, pkt_hdr, sizeof(AaruPacketHeader), MSG_PEEK);

            if(recv_size < 0)
            {
                printf("Error %d reading response from client, closing connection...\n", errno);
                NetClose(cli_ctx);
                free(pkt_hdr);
                break;
            }

            if(recv_size == 0)
            {
                printf("Client closed connection, closing connection...\n");
                NetClose(cli_ctx);
                free(pkt_hdr);
                break;
            }

            if(pkt_hdr->remote_id != htole32(AARUREMOTE_REMOTE_ID) ||
               pkt_hdr->packet_id != htole32(AARUREMOTE_PACKET_ID))
            {
                printf("Received data is not a correct aaruremote packet, closing connection...\n");
                NetClose(cli_ctx);
                free(pkt_hdr);
                break;
            }

            if(pkt_hdr->version != AARUREMOTE_PACKET_VERSION)
            {
                printf("Unrecognized packet version, skipping...\n");
                skip_next_hdr = 1;
                continue;
            }

            switch(pkt_hdr->packet_type)
            {
                case AARUREMOTE_PACKET_TYPE_HELLO:
                    pkt_nop->reason_code = AARUREMOTE_PACKET_NOP_REASON_OOO;
                    memset(&pkt_nop->reason, 0, 256);
                    strncpy(pkt_nop->reason, "Received hello packet out of order, skipping...", 256);
                    NetWrite(cli_ctx, pkt_nop, sizeof(AaruPacketNop));
                    printf("%s...\n", pkt_nop->reason);
                    skip_next_hdr = 1;
                    continue;
                case AARUREMOTE_PACKET_TYPE_COMMAND_LIST_DEVICES:
                    device_info_list = ListDevices();

                    // Packet only contains header so, dummy
                    in_buf = malloc(le32toh(pkt_hdr->len));

                    if(!in_buf)
                    {
                        printf("Fatal error %d allocating memory for packet, closing connection...\n", errno);
                        free(pkt_hdr);
                        NetClose(cli_ctx);
                        continue;
                    }

                    NetRecv(cli_ctx, in_buf, le32toh(pkt_hdr->len), 0);
                    free(in_buf);

                    if(!device_info_list)
                    {
                        pkt_nop->reason_code = AARUREMOTE_PACKET_NOP_REASON_ERROR_LIST_DEVICES;
                        memset(&pkt_nop->reason, 0, 256);
                        strncpy(pkt_nop->reason, "Could not get device list, continuing...", 256);
                        NetWrite(cli_ctx, pkt_nop, sizeof(AaruPacketNop));
                        printf("%s...\n", pkt_nop->reason);
                        continue;
                    }

                    pkt_res_devinfo          = malloc(sizeof(AaruPacketResListDevs));
                    pkt_res_devinfo->devices = htole16(DeviceInfoListCount(device_info_list));

                    n      = sizeof(AaruPacketResListDevs) + le16toh(pkt_res_devinfo->devices) * sizeof(DeviceInfo);
                    in_buf = malloc(n);
                    ((AaruPacketResListDevs*)in_buf)->hdr.len = htole32(n);
                    ((AaruPacketResListDevs*)in_buf)->devices = pkt_res_devinfo->devices;
                    free(pkt_res_devinfo);
                    pkt_res_devinfo = (AaruPacketResListDevs*)in_buf;

                    pkt_res_devinfo->hdr.remote_id   = htole32(AARUREMOTE_REMOTE_ID);
                    pkt_res_devinfo->hdr.packet_id   = htole32(AARUREMOTE_PACKET_ID);
                    pkt_res_devinfo->hdr.version     = AARUREMOTE_PACKET_VERSION;
                    pkt_res_devinfo->hdr.packet_type = AARUREMOTE_PACKET_TYPE_RESPONSE_LIST_DEVICES;

                    // Save list start
                    in_buf = (char*)device_info_list;
                    off    = sizeof(AaruPacketResListDevs);

                    while(device_info_list)
                    {
                        memcpy(((char*)pkt_res_devinfo) + off, &device_info_list->this, sizeof(DeviceInfo));
                        device_info_list = device_info_list->next;
                        off += sizeof(DeviceInfo);
                    }

                    device_info_list = (struct DeviceInfoList*)in_buf;
                    FreeDeviceInfoList(device_info_list);

                    NetWrite(cli_ctx, pkt_res_devinfo, le32toh(pkt_res_devinfo->hdr.len));
                    free(pkt_res_devinfo);
                    continue;
                case AARUREMOTE_PACKET_TYPE_RESPONSE_GET_SDHCI_REGISTERS:
                case AARUREMOTE_PACKET_TYPE_RESPONSE_LIST_DEVICES:
                case AARUREMOTE_PACKET_TYPE_RESPONSE_SCSI:
                case AARUREMOTE_PACKET_TYPE_RESPONSE_ATA_CHS:
                case AARUREMOTE_PACKET_TYPE_RESPONSE_ATA_LBA_28:
                case AARUREMOTE_PACKET_TYPE_RESPONSE_ATA_LBA_48:
                case AARUREMOTE_PACKET_TYPE_RESPONSE_SDHCI:
                case AARUREMOTE_PACKET_TYPE_RESPONSE_GET_DEVTYPE:
                case AARUREMOTE_PACKET_TYPE_RESPONSE_GET_USB_DATA:
                case AARUREMOTE_PACKET_TYPE_RESPONSE_GET_FIREWIRE_DATA:
                case AARUREMOTE_PACKET_TYPE_RESPONSE_GET_PCMCIA_DATA:
                    pkt_nop->reason_code = AARUREMOTE_PACKET_NOP_REASON_OOO;
                    memset(&pkt_nop->reason, 0, 256);
                    strncpy(pkt_nop->reason, "Received response packet?! You should certainly not do that...", 256);
                    NetWrite(cli_ctx, pkt_nop, sizeof(AaruPacketNop));
                    printf("%s...\n", pkt_nop->reason);
                    skip_next_hdr = 1;
                    continue;
                case AARUREMOTE_PACKET_TYPE_COMMAND_OPEN_DEVICE:
                    pkt_dev_open = malloc(le32toh(pkt_hdr->len));

                    if(!pkt_dev_open)
                    {
                        printf("Fatal error %d allocating memory for packet, closing connection...\n", errno);
                        free(pkt_hdr);
                        NetClose(cli_ctx);
                        continue;
                    }

                    NetRecv(cli_ctx, pkt_dev_open, le32toh(pkt_hdr->len), 0);

                    device_ctx = DeviceOpen(pkt_dev_open->device_path);

                    pkt_nop->reason_code = device_ctx == NULL ? AARUREMOTE_PACKET_NOP_REASON_OPEN_ERROR
                                                              : AARUREMOTE_PACKET_NOP_REASON_OPEN_OK;
                    pkt_nop->error_no    = errno;
                    memset(&pkt_nop->reason, 0, 256);
                    NetWrite(cli_ctx, pkt_nop, sizeof(AaruPacketNop));

                    free(pkt_dev_open);
                    continue;
                case AARUREMOTE_PACKET_TYPE_COMMAND_GET_DEVTYPE:
                    // Packet only contains header so, dummy
                    in_buf = malloc(le32toh(pkt_hdr->len));

                    if(!in_buf)
                    {
                        printf("Fatal error %d allocating memory for packet, closing connection...\n", errno);
                        free(pkt_hdr);
                        NetClose(cli_ctx);
                        continue;
                    }

                    NetRecv(cli_ctx, in_buf, le32toh(pkt_hdr->len), 0);
                    free(in_buf);

                    pkt_dev_type = malloc(sizeof(AaruPacketResGetDeviceType));

                    if(!pkt_dev_type)
                    {
                        printf("Fatal error %d allocating memory for packet, closing connection...\n", errno);
                        free(pkt_hdr);
                        NetClose(cli_ctx);
                        continue;
                    }

                    memset(pkt_dev_type, 0, sizeof(AaruPacketResGetDeviceType));

                    pkt_dev_type->hdr.len         = htole32(sizeof(AaruPacketResGetDeviceType));
                    pkt_dev_type->hdr.packet_type = AARUREMOTE_PACKET_TYPE_RESPONSE_GET_DEVTYPE;
                    pkt_dev_type->hdr.version     = AARUREMOTE_PACKET_VERSION;
                    pkt_dev_type->hdr.remote_id   = htole32(AARUREMOTE_REMOTE_ID);
                    pkt_dev_type->hdr.packet_id   = htole32(AARUREMOTE_PACKET_ID);
                    pkt_dev_type->device_type     = htole32(GetDeviceType(device_ctx));

                    NetWrite(cli_ctx, pkt_dev_type, sizeof(AaruPacketResGetDeviceType));
                    free(pkt_dev_type);
                    continue;
                case AARUREMOTE_PACKET_TYPE_COMMAND_SCSI:
                    // Packet contains data after
                    in_buf = malloc(le32toh(pkt_hdr->len));

                    if(!in_buf)
                    {
                        printf("Fatal error %d allocating memory for packet, closing connection...\n", errno);
                        free(pkt_hdr);
                        NetClose(cli_ctx);
                        continue;
                    }

                    NetRecv(cli_ctx, in_buf, le32toh(pkt_hdr->len), 0);

                    pkt_cmd_scsi = (AaruPacketCmdScsi*)in_buf;

                    // TODO: Check size of buffers + size of packet is not bigger than size in header

                    if(le32toh(pkt_cmd_scsi->cdb_len) > 0)
                    {
                        cdb_buf = malloc(le32toh(pkt_cmd_scsi->cdb_len));
                        memcpy(cdb_buf, in_buf + sizeof(AaruPacketCmdScsi), le32toh(pkt_cmd_scsi->cdb_len));
                    }
                    else
                        cdb_buf = NULL;

                    if(le32toh(pkt_cmd_scsi->buf_len) > 0)
                    {
                        buffer = malloc(le32toh(pkt_cmd_scsi->buf_len));
                        memcpy(buffer,
                               in_buf + le32toh(pkt_cmd_scsi->cdb_len) + sizeof(AaruPacketCmdScsi),
                               le32toh(pkt_cmd_scsi->buf_len));
                    }
                    else
                        buffer = NULL;

                    // Swap buf_len
                    pkt_cmd_scsi->buf_len = le32toh(pkt_cmd_scsi->buf_len);

                    ret = SendScsiCommand(device_ctx,
                                          cdb_buf,
                                          buffer,
                                          &sense_buf,
                                          le32toh(pkt_cmd_scsi->timeout),
                                          le32toh(pkt_cmd_scsi->direction),
                                          &duration,
                                          &sense,
                                          le32toh(pkt_cmd_scsi->cdb_len),
                                          &pkt_cmd_scsi->buf_len,
                                          &sense_len);

                    // Swap buf_len back
                    pkt_cmd_scsi->buf_len = htole32(pkt_cmd_scsi->buf_len);

                    out_buf = malloc(sizeof(AaruPacketResScsi) + sense_len + le32toh(pkt_cmd_scsi->buf_len));

                    if(!out_buf)
                    {
                        printf("Fatal error %d allocating memory for packet, continuing...\n", errno);
                        free(pkt_hdr);
                        free(in_buf);
                        NetClose(cli_ctx);
                        continue;
                    }

                    pkt_res_scsi = (AaruPacketResScsi*)out_buf;
                    if(sense_buf) memcpy(out_buf + sizeof(AaruPacketResScsi), sense_buf, sense_len);
                    if(buffer) memcpy(out_buf + sizeof(AaruPacketResScsi) + sense_len, buffer, pkt_cmd_scsi->buf_len);

                    pkt_res_scsi->hdr.len = htole32(sizeof(AaruPacketResScsi) + sense_len + pkt_cmd_scsi->buf_len);
                    pkt_res_scsi->hdr.packet_type = AARUREMOTE_PACKET_TYPE_RESPONSE_SCSI;
                    pkt_res_scsi->hdr.version     = AARUREMOTE_PACKET_VERSION;
                    pkt_res_scsi->hdr.remote_id   = htole32(AARUREMOTE_REMOTE_ID);
                    pkt_res_scsi->hdr.packet_id   = htole32(AARUREMOTE_PACKET_ID);

                    pkt_res_scsi->sense_len = htole32(sense_len);
                    pkt_res_scsi->buf_len   = pkt_cmd_scsi->buf_len;
                    pkt_res_scsi->duration  = htole32(duration);
                    pkt_res_scsi->sense     = htole32(sense);
                    pkt_res_scsi->error_no  = htole32(ret);

                    NetWrite(cli_ctx, pkt_res_scsi, le32toh(pkt_res_scsi->hdr.len));
                    free(pkt_cmd_scsi);
                    free(pkt_res_scsi);
                    if(cdb_buf) free(cdb_buf);
                    if(buffer) free(buffer);
                    if(sense_buf) free(sense_buf);
                    continue;
                case AARUREMOTE_PACKET_TYPE_COMMAND_GET_SDHCI_REGISTERS:
                    // Packet only contains header so, dummy
                    in_buf = malloc(le32toh(pkt_hdr->len));

                    if(!in_buf)
                    {
                        printf("Fatal error %d allocating memory for packet, closing connection...\n", errno);
                        free(pkt_hdr);
                        NetClose(cli_ctx);
                        continue;
                    }

                    NetRecv(cli_ctx, in_buf, le32toh(pkt_hdr->len), 0);
                    free(in_buf);

                    pkt_res_sdhci_registers = malloc(sizeof(AaruPacketResGetSdhciRegisters));
                    if(!pkt_res_sdhci_registers)
                    {
                        printf("Fatal error %d allocating memory for packet, closing connection...\n", errno);
                        free(pkt_hdr);
                        NetClose(cli_ctx);
                        continue;
                    }

                    memset(pkt_res_sdhci_registers, 0, sizeof(AaruPacketResGetSdhciRegisters));
                    pkt_res_sdhci_registers->hdr.remote_id   = htole32(AARUREMOTE_REMOTE_ID);
                    pkt_res_sdhci_registers->hdr.packet_id   = htole32(AARUREMOTE_PACKET_ID);
                    pkt_res_sdhci_registers->hdr.version     = AARUREMOTE_PACKET_VERSION;
                    pkt_res_sdhci_registers->hdr.packet_type = AARUREMOTE_PACKET_TYPE_RESPONSE_GET_SDHCI_REGISTERS;
                    pkt_res_sdhci_registers->hdr.len         = htole32(sizeof(AaruPacketResGetSdhciRegisters));
                    pkt_res_sdhci_registers->is_sdhci        = GetSdhciRegisters(device_ctx,
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

                    // Swap lengths
                    pkt_res_sdhci_registers->csd_len = htole32(pkt_res_sdhci_registers->csd_len);
                    pkt_res_sdhci_registers->cid_len = htole32(pkt_res_sdhci_registers->cid_len);
                    pkt_res_sdhci_registers->ocr_len = htole32(pkt_res_sdhci_registers->ocr_len);
                    pkt_res_sdhci_registers->scr_len = htole32(pkt_res_sdhci_registers->scr_len);

                    free(csd);
                    free(cid);
                    free(scr);
                    free(ocr);

                    NetWrite(cli_ctx, pkt_res_sdhci_registers, le32toh(pkt_res_sdhci_registers->hdr.len));
                    free(pkt_res_sdhci_registers);
                    continue;
                case AARUREMOTE_PACKET_TYPE_COMMAND_GET_USB_DATA:
                    // Packet only contains header so, dummy
                    in_buf = malloc(le32toh(pkt_hdr->len));

                    if(!in_buf)
                    {
                        printf("Fatal error %d allocating memory for packet, closing connection...\n", errno);
                        free(pkt_hdr);
                        NetClose(cli_ctx);
                        continue;
                    }

                    NetRecv(cli_ctx, in_buf, le32toh(pkt_hdr->len), 0);
                    free(in_buf);

                    pkt_res_usb = malloc(sizeof(AaruPacketResGetUsbData));
                    if(!pkt_res_usb)
                    {
                        printf("Fatal error %d allocating memory for packet, closing connection...\n", errno);
                        free(pkt_hdr);
                        NetClose(cli_ctx);
                        continue;
                    }

                    memset(pkt_res_usb, 0, sizeof(AaruPacketResGetUsbData));
                    pkt_res_usb->hdr.remote_id   = htole32(AARUREMOTE_REMOTE_ID);
                    pkt_res_usb->hdr.packet_id   = htole32(AARUREMOTE_PACKET_ID);
                    pkt_res_usb->hdr.version     = AARUREMOTE_PACKET_VERSION;
                    pkt_res_usb->hdr.packet_type = AARUREMOTE_PACKET_TYPE_RESPONSE_GET_USB_DATA;
                    pkt_res_usb->hdr.len         = htole32(sizeof(AaruPacketResGetUsbData));
                    pkt_res_usb->is_usb          = GetUsbData(device_ctx,
                                                     &pkt_res_usb->desc_len,
                                                     pkt_res_usb->descriptors,
                                                     &pkt_res_usb->id_vendor,
                                                     &pkt_res_usb->id_product,
                                                     pkt_res_usb->manufacturer,
                                                     pkt_res_usb->product,
                                                     pkt_res_usb->serial);

                    // Swap parameters
                    pkt_res_usb->desc_len = htole32(pkt_res_usb->desc_len);
                    // TODO: Need to swap vendor, product?

                    NetWrite(cli_ctx, pkt_res_usb, le32toh(pkt_res_usb->hdr.len));
                    free(pkt_res_usb);
                    continue;
                case AARUREMOTE_PACKET_TYPE_COMMAND_GET_FIREWIRE_DATA:
                    // Packet only contains header so, dummy
                    in_buf = malloc(le32toh(pkt_hdr->len));

                    if(!in_buf)
                    {
                        printf("Fatal error %d allocating memory for packet, closing connection...\n", errno);
                        free(pkt_hdr);
                        NetClose(cli_ctx);
                        continue;
                    }

                    NetRecv(cli_ctx, in_buf, le32toh(pkt_hdr->len), 0);
                    free(in_buf);

                    pkt_res_firewire = malloc(sizeof(AaruPacketResGetFireWireData));
                    if(!pkt_res_firewire)
                    {
                        printf("Fatal error %d allocating memory for packet, closing connection...\n", errno);
                        free(pkt_hdr);
                        NetClose(cli_ctx);
                        continue;
                    }

                    memset(pkt_res_firewire, 0, sizeof(AaruPacketResGetFireWireData));
                    pkt_res_firewire->hdr.remote_id   = htole32(AARUREMOTE_REMOTE_ID);
                    pkt_res_firewire->hdr.packet_id   = htole32(AARUREMOTE_PACKET_ID);
                    pkt_res_firewire->hdr.version     = AARUREMOTE_PACKET_VERSION;
                    pkt_res_firewire->hdr.packet_type = AARUREMOTE_PACKET_TYPE_RESPONSE_GET_FIREWIRE_DATA;
                    pkt_res_firewire->hdr.len         = htole32(sizeof(AaruPacketResGetFireWireData));
                    pkt_res_firewire->is_firewire     = GetFireWireData(device_ctx,
                                                                    &pkt_res_firewire->id_model,
                                                                    &pkt_res_firewire->id_vendor,
                                                                    &pkt_res_firewire->guid,
                                                                    pkt_res_firewire->vendor,
                                                                    pkt_res_firewire->model);

                    // TODO: Need to swap IDs?

                    NetWrite(cli_ctx, pkt_res_firewire, le32toh(pkt_res_firewire->hdr.len));
                    free(pkt_res_firewire);
                    continue;
                case AARUREMOTE_PACKET_TYPE_COMMAND_GET_PCMCIA_DATA:
                    // Packet only contains header so, dummy
                    in_buf = malloc(le32toh(pkt_hdr->len));

                    if(!in_buf)
                    {
                        printf("Fatal error %d allocating memory for packet, closing connection...\n", errno);
                        free(pkt_hdr);
                        NetClose(cli_ctx);
                        continue;
                    }

                    NetRecv(cli_ctx, in_buf, le32toh(pkt_hdr->len), 0);
                    free(in_buf);

                    pkt_res_pcmcia = malloc(sizeof(AaruPacketResGetPcmciaData));
                    if(!pkt_res_pcmcia)
                    {
                        printf("Fatal error %d allocating memory for packet, closing connection...\n", errno);
                        free(pkt_hdr);
                        NetClose(cli_ctx);
                        continue;
                    }

                    memset(pkt_res_pcmcia, 0, sizeof(AaruPacketResGetPcmciaData));
                    pkt_res_pcmcia->hdr.remote_id   = htole32(AARUREMOTE_REMOTE_ID);
                    pkt_res_pcmcia->hdr.packet_id   = htole32(AARUREMOTE_PACKET_ID);
                    pkt_res_pcmcia->hdr.version     = AARUREMOTE_PACKET_VERSION;
                    pkt_res_pcmcia->hdr.packet_type = AARUREMOTE_PACKET_TYPE_RESPONSE_GET_PCMCIA_DATA;
                    pkt_res_pcmcia->hdr.len         = htole32(sizeof(AaruPacketResGetPcmciaData));
                    pkt_res_pcmcia->is_pcmcia =
                        GetPcmciaData(device_ctx, &pkt_res_pcmcia->cis_len, pkt_res_pcmcia->cis);

                    pkt_res_pcmcia->cis_len = htole32(pkt_res_pcmcia->cis_len);

                    NetWrite(cli_ctx, pkt_res_pcmcia, le32toh(pkt_res_pcmcia->hdr.len));
                    free(pkt_res_pcmcia);
                    continue;
                case AARUREMOTE_PACKET_TYPE_COMMAND_ATA_CHS:
                    // Packet contains data after
                    in_buf = malloc(le32toh(pkt_hdr->len));

                    if(!in_buf)
                    {
                        printf("Fatal error %d allocating memory for packet, closing connection...\n", errno);
                        free(pkt_hdr);
                        NetClose(cli_ctx);
                        continue;
                    }

                    NetRecv(cli_ctx, in_buf, le32toh(pkt_hdr->len), 0);

                    pkt_cmd_ata_chs = (AaruPacketCmdAtaChs*)in_buf;

                    // TODO: Check size of buffers + size of packet is not bigger than size in header

                    if(le32toh(pkt_cmd_ata_chs->buf_len) > 0) buffer = in_buf + sizeof(AaruPacketCmdAtaChs);
                    else
                        buffer = NULL;

                    memset(&ata_chs_error_regs, 0, sizeof(AtaErrorRegistersChs));

                    pkt_cmd_ata_chs->buf_len = le32toh(pkt_cmd_ata_chs->buf_len);

                    duration = 0;
                    sense    = 1;
                    ret      = SendAtaChsCommand(device_ctx,
                                            pkt_cmd_ata_chs->registers,
                                            &ata_chs_error_regs,
                                            pkt_cmd_ata_chs->protocol,
                                            pkt_cmd_ata_chs->transfer_register,
                                            buffer,
                                            le32toh(pkt_cmd_ata_chs->timeout),
                                            pkt_cmd_ata_chs->transfer_blocks,
                                            &duration,
                                            &sense,
                                            &pkt_cmd_ata_chs->buf_len);

                    out_buf = malloc(sizeof(AaruPacketResAtaChs) + pkt_cmd_ata_chs->buf_len);

                    pkt_cmd_ata_chs->buf_len = htole32(pkt_cmd_ata_chs->buf_len);

                    if(!out_buf)
                    {
                        printf("Fatal error %d allocating memory for packet, continuing...\n", errno);
                        free(pkt_hdr);
                        free(in_buf);
                        NetClose(cli_ctx);
                        continue;
                    }

                    pkt_res_ata_chs = (AaruPacketResAtaChs*)out_buf;
                    if(buffer) memcpy(out_buf + sizeof(AaruPacketResAtaChs), buffer, htole32(pkt_cmd_ata_chs->buf_len));

                    pkt_res_ata_chs->hdr.len = htole32(sizeof(AaruPacketResAtaChs) + htole32(pkt_cmd_ata_chs->buf_len));
                    pkt_res_ata_chs->hdr.packet_type = AARUREMOTE_PACKET_TYPE_RESPONSE_ATA_CHS;
                    pkt_res_ata_chs->hdr.version     = AARUREMOTE_PACKET_VERSION;
                    pkt_res_ata_chs->hdr.remote_id   = htole32(AARUREMOTE_REMOTE_ID);
                    pkt_res_ata_chs->hdr.packet_id   = htole32(AARUREMOTE_PACKET_ID);

                    pkt_res_ata_chs->registers = ata_chs_error_regs;
                    pkt_res_ata_chs->buf_len   = pkt_cmd_ata_chs->buf_len;
                    pkt_res_ata_chs->duration  = htole32(duration);
                    pkt_res_ata_chs->sense     = htole32(sense);
                    pkt_res_ata_chs->error_no  = htole32(ret);

                    NetWrite(cli_ctx, pkt_res_ata_chs, le32toh(pkt_res_ata_chs->hdr.len));
                    free(pkt_cmd_ata_chs);
                    free(pkt_res_ata_chs);
                    continue;
                case AARUREMOTE_PACKET_TYPE_COMMAND_ATA_LBA_28:
                    // Packet contains data after
                    in_buf = malloc(le32toh(pkt_hdr->len));

                    if(!in_buf)
                    {
                        printf("Fatal error %d allocating memory for packet, closing connection...\n", errno);
                        free(pkt_hdr);
                        NetClose(cli_ctx);
                        continue;
                    }

                    NetRecv(cli_ctx, in_buf, le32toh(pkt_hdr->len), 0);

                    pkt_cmd_ata_lba28 = (AaruPacketCmdAtaLba28*)in_buf;

                    // TODO: Check size of buffers + size of packet is not bigger than size in header

                    if(le32toh(pkt_cmd_ata_lba28->buf_len) > 0) buffer = in_buf + sizeof(AaruPacketCmdAtaLba28);
                    else
                        buffer = NULL;

                    memset(&ata_lba28_error_regs, 0, sizeof(AtaErrorRegistersLba28));

                    pkt_cmd_ata_lba28->buf_len = le32toh(pkt_cmd_ata_lba28->buf_len);

                    duration = 0;
                    sense    = 1;
                    ret      = SendAtaLba28Command(device_ctx,
                                              pkt_cmd_ata_lba28->registers,
                                              &ata_lba28_error_regs,
                                              pkt_cmd_ata_lba28->protocol,
                                              pkt_cmd_ata_lba28->transfer_register,
                                              buffer,
                                              le32toh(pkt_cmd_ata_lba28->timeout),
                                              pkt_cmd_ata_lba28->transfer_blocks,
                                              &duration,
                                              &sense,
                                              &pkt_cmd_ata_lba28->buf_len);

                    out_buf                    = malloc(sizeof(AaruPacketResAtaLba28) + pkt_cmd_ata_lba28->buf_len);
                    pkt_cmd_ata_lba28->buf_len = htole32(pkt_cmd_ata_lba28->buf_len);

                    if(!out_buf)
                    {
                        printf("Fatal error %d allocating memory for packet, continuing...\n", errno);
                        free(pkt_hdr);
                        free(in_buf);
                        NetClose(cli_ctx);
                        continue;
                    }

                    pkt_res_ata_lba28 = (AaruPacketResAtaLba28*)out_buf;
                    if(buffer)
                        memcpy(out_buf + sizeof(AaruPacketResAtaLba28), buffer, le32toh(pkt_cmd_ata_lba28->buf_len));

                    pkt_res_ata_lba28->hdr.len =
                        htole32(sizeof(AaruPacketResAtaLba28) + le32toh(pkt_cmd_ata_lba28->buf_len));
                    pkt_res_ata_lba28->hdr.packet_type = AARUREMOTE_PACKET_TYPE_RESPONSE_ATA_LBA_28;
                    pkt_res_ata_lba28->hdr.version     = AARUREMOTE_PACKET_VERSION;
                    pkt_res_ata_lba28->hdr.remote_id   = htole32(AARUREMOTE_REMOTE_ID);
                    pkt_res_ata_lba28->hdr.packet_id   = htole32(AARUREMOTE_PACKET_ID);

                    pkt_res_ata_lba28->registers = ata_lba28_error_regs;
                    pkt_res_ata_lba28->buf_len   = pkt_cmd_ata_lba28->buf_len;
                    pkt_res_ata_lba28->duration  = le32toh(duration);
                    pkt_res_ata_lba28->sense     = le32toh(sense);
                    pkt_res_ata_lba28->error_no  = le32toh(ret);

                    NetWrite(cli_ctx, pkt_res_ata_lba28, le32toh(pkt_res_ata_lba28->hdr.len));
                    free(pkt_cmd_ata_lba28);
                    free(pkt_res_ata_lba28);
                    continue;
                case AARUREMOTE_PACKET_TYPE_COMMAND_ATA_LBA_48:
                    // Packet contains data after
                    in_buf = malloc(le32toh(pkt_hdr->len));

                    if(!in_buf)
                    {
                        printf("Fatal error %d allocating memory for packet, closing connection...\n", errno);
                        free(pkt_hdr);
                        NetClose(cli_ctx);
                        continue;
                    }

                    NetRecv(cli_ctx, in_buf, le32toh(pkt_hdr->len), 0);

                    pkt_cmd_ata_lba48 = (AaruPacketCmdAtaLba48*)in_buf;

                    // TODO: Check size of buffers + size of packet is not bigger than size in header

                    if(le32toh(pkt_cmd_ata_lba48->buf_len) > 0) buffer = in_buf + sizeof(AaruPacketCmdAtaLba48);
                    else
                        buffer = NULL;

                    memset(&ata_lba48_error_regs, 0, sizeof(AtaErrorRegistersLba48));
                    pkt_cmd_ata_lba48->buf_len = le32toh(pkt_cmd_ata_lba48->buf_len);

                    // Swapping
                    pkt_cmd_ata_lba48->registers.lba_high     = le16toh(pkt_cmd_ata_lba48->registers.lba_high);
                    pkt_cmd_ata_lba48->registers.lba_mid      = le16toh(pkt_cmd_ata_lba48->registers.lba_mid);
                    pkt_cmd_ata_lba48->registers.lba_low      = le16toh(pkt_cmd_ata_lba48->registers.lba_low);
                    pkt_cmd_ata_lba48->registers.sector_count = le16toh(pkt_cmd_ata_lba48->registers.sector_count);

                    duration = 0;
                    sense    = 1;
                    ret      = SendAtaLba48Command(device_ctx,
                                              pkt_cmd_ata_lba48->registers,
                                              &ata_lba48_error_regs,
                                              pkt_cmd_ata_lba48->protocol,
                                              pkt_cmd_ata_lba48->transfer_register,
                                              buffer,
                                              le32toh(pkt_cmd_ata_lba48->timeout),
                                              pkt_cmd_ata_lba48->transfer_blocks,
                                              &duration,
                                              &sense,
                                              &pkt_cmd_ata_lba48->buf_len);

                    out_buf                    = malloc(sizeof(AaruPacketResAtaLba48) + pkt_cmd_ata_lba48->buf_len);
                    pkt_cmd_ata_lba48->buf_len = htole32(pkt_cmd_ata_lba48->buf_len);

                    if(!out_buf)
                    {
                        printf("Fatal error %d allocating memory for packet, continuing...\n", errno);
                        free(pkt_hdr);
                        free(in_buf);
                        NetClose(cli_ctx);
                        continue;
                    }

                    pkt_res_ata_lba48 = (AaruPacketResAtaLba48*)out_buf;
                    if(buffer)
                        memcpy(out_buf + sizeof(AaruPacketResAtaLba48), buffer, le32toh(pkt_cmd_ata_lba48->buf_len));

                    pkt_res_ata_lba48->hdr.len =
                        htole32(sizeof(AaruPacketResAtaLba48) + le32toh(pkt_cmd_ata_lba48->buf_len));
                    pkt_res_ata_lba48->hdr.packet_type = AARUREMOTE_PACKET_TYPE_RESPONSE_ATA_LBA_48;
                    pkt_res_ata_lba48->hdr.version     = AARUREMOTE_PACKET_VERSION;
                    pkt_res_ata_lba48->hdr.remote_id   = htole32(AARUREMOTE_REMOTE_ID);
                    pkt_res_ata_lba48->hdr.packet_id   = htole32(AARUREMOTE_PACKET_ID);

                    // Swapping
                    ata_lba48_error_regs.lba_high     = htole16(ata_lba48_error_regs.lba_high);
                    ata_lba48_error_regs.lba_mid      = htole16(ata_lba48_error_regs.lba_mid);
                    ata_lba48_error_regs.lba_low      = htole16(ata_lba48_error_regs.lba_low);
                    ata_lba48_error_regs.sector_count = htole16(ata_lba48_error_regs.sector_count);

                    pkt_res_ata_lba48->registers = ata_lba48_error_regs;
                    pkt_res_ata_lba48->buf_len   = pkt_cmd_ata_lba48->buf_len;
                    pkt_res_ata_lba48->duration  = le32toh(duration);
                    pkt_res_ata_lba48->sense     = le32toh(sense);
                    pkt_res_ata_lba48->error_no  = le32toh(ret);

                    NetWrite(cli_ctx, pkt_res_ata_lba48, le32toh(pkt_res_ata_lba48->hdr.len));
                    free(pkt_cmd_ata_lba48);
                    free(pkt_res_ata_lba48);
                    continue;
                case AARUREMOTE_PACKET_TYPE_COMMAND_SDHCI:
                    // Packet contains data after
                    in_buf = malloc(le32toh(pkt_hdr->len));

                    if(!in_buf)
                    {
                        printf("Fatal error %d allocating memory for packet, closing connection...\n", errno);
                        free(pkt_hdr);
                        NetClose(cli_ctx);
                        continue;
                    }

                    NetRecv(cli_ctx, in_buf, le32toh(pkt_hdr->len), 0);

                    pkt_cmd_sdhci = (AaruPacketCmdSdhci*)in_buf;

                    // TODO: Check size of buffers + size of packet is not bigger than size in header

                    if(le32toh(pkt_cmd_sdhci->buf_len) > 0) buffer = in_buf + sizeof(AaruPacketCmdSdhci);
                    else
                        buffer = NULL;

                    memset((char*)&sdhci_response, 0, sizeof(uint32_t) * 4);

                    duration = 0;
                    sense    = 1;
                    ret      = SendSdhciCommand(device_ctx,
                                           pkt_cmd_sdhci->command,
                                           pkt_cmd_sdhci->write,
                                           pkt_cmd_sdhci->application,
                                           le32toh(pkt_cmd_sdhci->flags),
                                           le32toh(pkt_cmd_sdhci->argument),
                                           le32toh(pkt_cmd_sdhci->block_size),
                                           le32toh(pkt_cmd_sdhci->blocks),
                                           buffer,
                                           le32toh(pkt_cmd_sdhci->buf_len),
                                           le32toh(pkt_cmd_sdhci->timeout),
                                           (uint32_t*)&sdhci_response,
                                           &duration,
                                           &sense);

                    out_buf = malloc(sizeof(AaruPacketResSdhci) + le32toh(pkt_cmd_sdhci->buf_len));

                    if(!out_buf)
                    {
                        printf("Fatal error %d allocating memory for packet, continuing...\n", errno);
                        free(pkt_hdr);
                        free(in_buf);
                        NetClose(cli_ctx);
                        continue;
                    }

                    pkt_res_sdhci = (AaruPacketResSdhci*)out_buf;
                    if(buffer) memcpy(out_buf + sizeof(AaruPacketResSdhci), buffer, le32toh(pkt_cmd_sdhci->buf_len));

                    pkt_res_sdhci->hdr.len = htole32(sizeof(AaruPacketResSdhci) + le32toh(pkt_cmd_sdhci->buf_len));
                    pkt_res_sdhci->hdr.packet_type = AARUREMOTE_PACKET_TYPE_RESPONSE_SDHCI;
                    pkt_res_sdhci->hdr.version     = AARUREMOTE_PACKET_VERSION;
                    pkt_res_sdhci->hdr.remote_id   = htole32(AARUREMOTE_REMOTE_ID);
                    pkt_res_sdhci->hdr.packet_id   = htole32(AARUREMOTE_PACKET_ID);

                    sdhci_response[0] = htole32(sdhci_response[0]);
                    sdhci_response[1] = htole32(sdhci_response[1]);
                    sdhci_response[2] = htole32(sdhci_response[2]);
                    sdhci_response[3] = htole32(sdhci_response[3]);

                    memcpy((char*)&pkt_res_sdhci->response, (char*)&sdhci_response, sizeof(uint32_t) * 4);
                    pkt_res_sdhci->buf_len  = pkt_cmd_sdhci->buf_len;
                    pkt_res_sdhci->duration = htole32(duration);
                    pkt_res_sdhci->sense    = htole32(sense);
                    pkt_res_sdhci->error_no = htole32(ret);

                    NetWrite(cli_ctx, pkt_res_sdhci, le32toh(pkt_res_sdhci->hdr.len));
                    free(pkt_cmd_sdhci);
                    free(pkt_res_sdhci);
                    continue;
                case AARUREMOTE_PACKET_TYPE_COMMAND_CLOSE_DEVICE:
                    DeviceClose(device_ctx);
                    device_ctx    = NULL;
                    skip_next_hdr = 1;
                    continue;
                case AARUREMOTE_PACKET_TYPE_COMMAND_AM_I_ROOT:
                    // Packet only contains header so, dummy
                    in_buf = malloc(le32toh(pkt_hdr->len));

                    if(!in_buf)
                    {
                        printf("Fatal error %d allocating memory for packet, closing connection...\n", errno);
                        free(pkt_hdr);
                        NetClose(cli_ctx);
                        continue;
                    }

                    NetRecv(cli_ctx, in_buf, le32toh(pkt_hdr->len), 0);
                    free(in_buf);

                    pkt_res_am_i_root = malloc(sizeof(AaruPacketResAmIRoot));
                    if(!pkt_res_am_i_root)
                    {
                        printf("Fatal error %d allocating memory for packet, closing connection...\n", errno);
                        free(pkt_hdr);
                        NetClose(cli_ctx);
                        continue;
                    }

                    memset(pkt_res_am_i_root, 0, sizeof(AaruPacketResAmIRoot));
                    pkt_res_am_i_root->hdr.remote_id   = htole32(AARUREMOTE_REMOTE_ID);
                    pkt_res_am_i_root->hdr.packet_id   = htole32(AARUREMOTE_PACKET_ID);
                    pkt_res_am_i_root->hdr.version     = AARUREMOTE_PACKET_VERSION;
                    pkt_res_am_i_root->hdr.packet_type = AARUREMOTE_PACKET_TYPE_RESPONSE_AM_I_ROOT;
                    pkt_res_am_i_root->hdr.len         = htole32(sizeof(AaruPacketResAmIRoot));
                    pkt_res_am_i_root->am_i_root       = AmIRoot();

                    NetWrite(cli_ctx, pkt_res_am_i_root, le32toh(pkt_res_am_i_root->hdr.len));
                    free(pkt_res_am_i_root);
                    continue;
                default:
                    pkt_nop->reason_code = AARUREMOTE_PACKET_NOP_REASON_NOT_RECOGNIZED;
                    memset(&pkt_nop->reason, 0, 256);
#ifdef _WIN32
                    sprintf_s(pkt_nop->reason,
                              256,
                              "Received unrecognized packet with type %d, skipping...",
                              pkt_hdr->packet_type);
#else
                    snprintf(pkt_nop->reason,
                             256,
                             "Received unrecognized packet with type %d, skipping...",
                             pkt_hdr->packet_type);
#endif
                    NetWrite(cli_ctx, pkt_nop, sizeof(AaruPacketNop));
                    printf("%s...\n", pkt_nop->reason);
                    skip_next_hdr = 1;
                    continue;
            }
        }
    }
    
    free(pkt_nop);
}
