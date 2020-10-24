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

#include <network.h>
#include <stdio.h>
#include <stdlib.h>

#include "../aaruremote.h"
#include "wii.h"

int PrintNetworkAddresses()
{
    int  ret;
    char localip[16] = {0};
    char gateway[16] = {0};
    char netmask[16] = {0};

    ret = if_config(localip, netmask, gateway, TRUE, 20);

    if(ret < 0) return -1;

    printf("Available addresses:\n");
    printf("%s port %d\n", localip, AARUREMOTE_PORT);

    return 0;
}

char* PrintIpv4Address(struct in_addr addr) { return inet_ntoa(addr); }
void* NetSocket(uint32_t domain, uint32_t type, uint32_t protocol)
{
    NetworkContext* ctx;

    ctx = malloc(sizeof(NetworkContext));

    if(!ctx) return NULL;

    ctx->fd = net_socket(domain, type, protocol);

    if(ctx->fd < 0)
    {
        free(ctx);
        return NULL;
    }

    return ctx;
}

int32_t NetBind(void* net_ctx, struct sockaddr* addr, socklen_t addrlen)
{
    NetworkContext* ctx = net_ctx;

    if(!ctx) return -1;

    return net_bind(ctx->fd, addr, addrlen);
}

int32_t NetListen(void* net_ctx, uint32_t backlog)
{
    NetworkContext* ctx = net_ctx;

    if(!ctx) return -1;

    return net_listen(ctx->fd, backlog);
}

void* NetAccept(void* net_ctx, struct sockaddr* addr, socklen_t* addrlen)
{
    NetworkContext* ctx = net_ctx;
    NetworkContext* cli_ctx;

    if(!ctx) return NULL;

    cli_ctx = malloc(sizeof(NetworkContext));

    if(!cli_ctx) return NULL;

    cli_ctx->fd = net_accept(ctx->fd, addr, addrlen);

    if(cli_ctx->fd < 0)
    {
        free(cli_ctx);
        return NULL;
    }

    return cli_ctx;
}

int32_t NetRecv(void* net_ctx, void* buf, int32_t len, uint32_t flags)
{
    NetworkContext* ctx = net_ctx;

    if(!ctx) return -1;

    int32_t got_once;
    int32_t got_total = 0;

    while(len > 0)
    {
        got_once = net_recv(ctx->fd, buf, len, flags);

        if(got_once <= 0) break;

        buf += got_once;
        got_total += got_once;
        len -= got_once;
    }

    return got_total;
}

int32_t NetWrite(void* net_ctx, const void* buf, int32_t size)
{
    NetworkContext* ctx = net_ctx;

    if(!ctx) return -1;

    return net_write(ctx->fd, buf, size);
}

int32_t NetClose(void* net_ctx)
{
    int             ret;
    NetworkContext* ctx = net_ctx;

    if(!ctx) return -1;

    ret = net_close(ctx->fd);
    free(ctx);
    return ret;
}