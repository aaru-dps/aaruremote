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

#include <network.h>
#include <stdio.h>

int PrintNetworkAddresses()
{
    int  ret;
    char localip[16] = {0};
    char gateway[16] = {0};
    char netmask[16] = {0};

    ret = if_config(localip, netmask, gateway, TRUE, 20);

    if(ret < 0) return -1;

    printf("Available addresses:\n");
    printf("%s port %d\n", localip, DICMOTE_PORT);

    return 0;
}

char*   PrintIpv4Address(struct in_addr addr) { return inet_ntoa(addr); }
int32_t NetSocket(uint32_t domain, uint32_t type, uint32_t protocol) { return net_socket(domain, type, protocol); }
int32_t NetBind(int32_t sockfd, struct sockaddr* addr, socklen_t addrlen) { return net_bind(sockfd, addr, addrlen); }
int32_t NetListen(int32_t sockfd, uint32_t backlog) { return net_listen(sockfd, backlog); }
int32_t NetAccept(int32_t sockfd, struct sockaddr* addr, socklen_t* addrlen)
{
    return net_accept(sockfd, addr, addrlen);
}
int32_t NetRecv(int32_t sockfd, void* buf, int32_t len, uint32_t flags)
{
    int32_t got_once;
    int32_t got_total = 0;

    while(len > 0)
    {
        got_once = net_recv(sockfd, buf, len, flags);

        if(got_once <= 0) break;

        buf += got_once;
        got_total += got_once;
        len -= got_once;
    }

    return got_total;
}
int32_t NetWrite(int32_t fd, const void* buf, int32_t size) { return net_write(fd, buf, size); }
int32_t NetClose(int32_t fd) { return net_close(fd); }
