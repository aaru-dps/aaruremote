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

#include "../aaruremote.h"
#include "win32.h"

#include <iphlpapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

int PrintNetworkAddresses()
{
    ULONG                       ret;
    ULONG                       family     = AF_INET;
    ULONG                       flags      = GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER;
    PIP_ADAPTER_ADDRESSES       pAddresses = NULL;
    PIP_ADAPTER_ADDRESSES       pCurrAddresses = NULL;
    PIP_ADAPTER_UNICAST_ADDRESS pUnicast       = NULL;
    ULONG                       outBufLen      = 32768;

    pAddresses = (IP_ADAPTER_ADDRESSES*)HeapAlloc(GetProcessHeap(), 0, outBufLen);

    if(pAddresses == NULL)
    {
        printf("Memory allocation failed for IP_ADAPTER_ADDRESSES struct\n");
        return -1;
    }

    ret = GetAdaptersAddresses(family, flags, NULL, pAddresses, &outBufLen);

    if(ret != NO_ERROR)
    {
        HeapFree(GetProcessHeap(), 0, pAddresses);

        return ret;
    }

    pCurrAddresses = pAddresses;

    printf("Available addresses:\n");

    while(pCurrAddresses)
    {
        pUnicast = pCurrAddresses->FirstUnicastAddress;

        while(pUnicast != NULL)
        {
            printf(
                "%s port %d\n", inet_ntoa(((struct sockaddr_in*)pUnicast->Address.lpSockaddr)->sin_addr), AARUREMOTE_PORT);
            pUnicast = pUnicast->Next;
        }

        pCurrAddresses = pCurrAddresses->Next;
    }

    HeapFree(GetProcessHeap(), 0, pAddresses);

    return 0;
}

char* PrintIpv4Address(struct in_addr addr) { return inet_ntoa(addr); }

void* NetSocket(uint32_t domain, uint32_t type, uint32_t protocol)
{
    WSADATA              ws;
    int                  ret;
    Win32NetworkContext* ctx;

    ret = WSAStartup(MAKEWORD(2, 0), &ws);

    if(ret)
    {
        printf("Error %d initializing WinSocks.", WSAGetLastError());
        return NULL;
    }

    ctx = malloc(sizeof(Win32NetworkContext));

    if(!ctx) return NULL;

    ctx->socket = socket(domain, type, protocol);

    if(ctx->socket == INVALID_SOCKET)
    {
        printf("Error %d creating socket", WSAGetLastError());
        free(ctx);
        return NULL;
    }

    return ctx;
}

int32_t NetBind(void* net_ctx, struct sockaddr* addr, socklen_t addrlen)
{
    Win32NetworkContext* ctx = net_ctx;

    if(!ctx) return -1;

    return bind(ctx->socket, addr, addrlen);
}

int32_t NetListen(void* net_ctx, uint32_t backlog)
{
    Win32NetworkContext* ctx = net_ctx;

    if(!ctx) return -1;

    return listen(ctx->socket, backlog);
}

void* NetAccept(void* net_ctx, struct sockaddr* addr, socklen_t* addrlen)
{
    Win32NetworkContext* ctx = net_ctx;
    Win32NetworkContext* cli_ctx;

    if(!ctx) return NULL;

    cli_ctx = malloc(sizeof(Win32NetworkContext));

    if(!cli_ctx) return NULL;

    cli_ctx->socket = accept(ctx->socket, addr, addrlen);

    if(cli_ctx->socket == INVALID_SOCKET)
    {
        free(cli_ctx);
        return NULL;
    }

    return cli_ctx;
}

int32_t NetRecv(void* net_ctx, void* buf, int32_t len, uint32_t flags)
{
    Win32NetworkContext* ctx     = net_ctx;
    char*                charbuf = buf;

    if(!ctx) return -1;

    int32_t got_once;
    int32_t got_total = 0;

    while(len > 0)
    {
        got_once = recv(ctx->socket, charbuf, len, flags);

        if(got_once <= 0) break;

        charbuf += got_once;
        got_total += got_once;
        len -= got_once;
    }

    return got_total;
}

int32_t NetWrite(void* net_ctx, const void* buf, int32_t size)
{
    Win32NetworkContext* ctx = net_ctx;

    if(!ctx) return -1;

    return send(ctx->socket, buf, size, 0);
}

int32_t NetClose(void* net_ctx)
{
    int                  ret;
    Win32NetworkContext* ctx = net_ctx;

    if(!ctx) return -1;

    ret = closesocket(ctx->socket);
    free(ctx);
    return ret;
}