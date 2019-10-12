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

int main()
{
    struct ifaddrs* ifa;
    struct ifaddrs* ifa_start;
    int             ret;
    char            ipv4Address[INET_ADDRSTRLEN];
    char            ipv6Address[INET6_ADDRSTRLEN];
    int sockfd;

    printf("DiscImageChef Remote Server %s\n", DICMOTE_VERSION);
    printf("Copyright (C) 2019 Natalia Portillo\n");

    ret = getifaddrs(&ifa);

    if(ret)
    {
        printf("Error %d enumerating interfaces\n", errno);
        return 1;
    }

    printf("Opening socket.\n");
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
    {
        printf("Error %d opening socket.\n", errno);
        return 1;
    }

    ifa_start=ifa;

    printf("Available addresses:\n");
    while(ifa != NULL)
    {
        if(ifa->ifa_addr)
        {
            if(ifa->ifa_addr->sa_family == AF_INET)
            {
                inet_ntop(AF_INET, &((struct sockaddr_in*)ifa->ifa_addr)->sin_addr, ipv4Address, INET_ADDRSTRLEN);
                printf("%s port 6666\n", ipv4Address);
            }
            else if(ifa->ifa_addr->sa_family == AF_INET6)
            {
                inet_ntop(AF_INET6, &((struct sockaddr_in6*)ifa->ifa_addr)->sin6_addr, ipv6Address, INET6_ADDRSTRLEN);
                printf("%s port 6666\n", ipv6Address);
            }
        }
        ifa = ifa->ifa_next;
    }

    freeifaddrs(ifa_start);

    printf("Closing socket");
    close(sockfd);

    return 0;
}