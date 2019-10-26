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
#include "../unix/unix.h"

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

char* PrintIpv4Address(struct in_addr addr) { return inet_ntoa(addr); }

void* NetSocket(uint32_t domain, uint32_t type, uint32_t protocol)
{
    UnixNetworkContext* ctx;

    ctx = malloc(sizeof(UnixNetworkContext));

    if(!ctx) return NULL;

    ctx->fd = socket(domain, type, protocol);

    if(ctx->fd < 0)
    {
        free(ctx);
        return NULL;
    }

    return ctx;
}

int32_t NetBind(void* net_ctx, struct sockaddr* addr, socklen_t addrlen)
{
    UnixNetworkContext* ctx = net_ctx;

    if(!ctx) return -1;

    return bind(ctx->fd, addr, addrlen);
}

int32_t NetListen(void* net_ctx, uint32_t backlog)
{
    UnixNetworkContext* ctx = net_ctx;

    if(!ctx) return -1;

    return listen(ctx->fd, backlog);
}

void* NetAccept(void* net_ctx, struct sockaddr* addr, socklen_t* addrlen)
{
    UnixNetworkContext* ctx = net_ctx;
    UnixNetworkContext* cli_ctx;

    if(!ctx) return NULL;

    cli_ctx = malloc(sizeof(UnixNetworkContext));

    if(!cli_ctx) return NULL;

    cli_ctx->fd = accept(ctx->fd, addr, addrlen);

    if(cli_ctx->fd < 0)
    {
        free(cli_ctx);
        return NULL;
    }

    return cli_ctx;
}

int32_t NetRecv(void* net_ctx, void* buf, int32_t len, uint32_t flags)
{
    UnixNetworkContext* ctx = net_ctx;

    if(!ctx) return -1;

    int32_t got_once;
    int32_t got_total = 0;

    while(len > 0)
    {
        got_once = recv(ctx->fd, buf, len, flags);

        if(got_once <= 0) break;

        buf += got_once;
        got_total += got_once;
        len -= got_once;
    }

    return got_total;
}

int32_t NetWrite(void* net_ctx, const void* buf, int32_t size)
{
    UnixNetworkContext* ctx = net_ctx;

    if(!ctx) return -1;

    return write(ctx->fd, buf, size);
}

int32_t NetClose(void* net_ctx)
{
    int                 ret;
    UnixNetworkContext* ctx = net_ctx;

    if(!ctx) return -1;

    ret = close(ctx->fd);
    free(ctx);
    return ret;
}
