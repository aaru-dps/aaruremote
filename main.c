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
    struct ifaddrs*    ifa;
    struct ifaddrs*    ifa_start;
    int                ret;
    char               ipv4Address[INET_ADDRSTRLEN];
    int                sockfd, cli_sock;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t          cli_len;

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

    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port        = htons(DICMOTE_PORT);

    if(bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("Error %d binding socket.\n", errno);
        close(sockfd);
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

    printf("Client connected successfully.\n");

    printf("Closing socket.\n");
    close(sockfd);

    return 0;
}