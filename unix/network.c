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

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <unistd.h>

int PrintNetworkAddresses()
{
    int             ret;
    struct ifaddrs* ifa;
    struct ifaddrs* ifa_start;
    char            ipv4_address[INET_ADDRSTRLEN];

    ret = getifaddrs(&ifa);

    if(ret) return -1;

    ifa_start = ifa;

    printf("Available addresses:\n");
    while(ifa != NULL)
    {
        if(ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET)
        {
            inet_ntop(AF_INET, &((struct sockaddr_in*)ifa->ifa_addr)->sin_addr, ipv4_address, INET_ADDRSTRLEN);
            printf("%s port %d\n", ipv4_address, DICMOTE_PORT);
        }

        ifa = ifa->ifa_next;
    }

    freeifaddrs(ifa_start);

    return 0;
}

char*   PrintIpv4Address(struct in_addr addr) { return inet_ntoa(addr); }
int32_t NetSocket(uint32_t domain, uint32_t type, uint32_t protocol) { return socket(domain, type, protocol); }
int32_t NetBind(int32_t sockfd, struct sockaddr* addr, socklen_t addrlen) { return bind(sockfd, addr, addrlen); }
int32_t NetListen(int32_t sockfd, uint32_t backlog) { return listen(sockfd, backlog); }
int32_t NetAccept(int32_t sockfd, struct sockaddr* addr, socklen_t* addrlen) { return accept(sockfd, addr, addrlen); }
int32_t NetRecv(int32_t sockfd, void* buf, int32_t len, uint32_t flags) { return recv(sockfd, buf, len, flags); }
int32_t NetWrite(int32_t fd, const void* buf, int32_t size) { return write(fd, buf, size); }
int32_t NetClose(int32_t fd) { return close(fd); }
