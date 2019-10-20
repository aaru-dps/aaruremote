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

int PrintNetworkAddresses()
{
    // TODO: Implement
    return -1;
}

char*   PrintIpv4Address(struct in_addr addr) { return inet_ntoa(addr); }
int32_t NetSocket(uint32_t domain, uint32_t type, uint32_t protocol) { return net_socket(domain, type, protocol); }
int32_t NetBind(int32_t sockfd, struct sockaddr* addr, socklen_t addrlen) { return net_bind(sockfd, addr, addrlen); }
int32_t NetListen(int32_t sockfd, uint32_t backlog) { return net_listen(sockfd, backlog); }
int32_t NetAccept(int32_t sockfd, struct sockaddr* addr, socklen_t* addrlen)
{
    return net_accept(sockfd, addr, addrlen);
}
int32_t NetRecv(int32_t sockfd, void* buf, int32_t len, uint32_t flags) { return net_recv(sockfd, buf, len, flags); }
int32_t NetWrite(int32_t fd, const void* buf, int32_t size) { return net_write(fd, buf, size); }
int32_t NetClose(int32_t fd) { return net_close(fd); }