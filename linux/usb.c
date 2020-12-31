/*
 * This file is part of the Aaru Remote Server.
 * Copyright (c) 2019-2021 Natalia Portillo.
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

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../aaruremote.h"
#include "linux.h"

uint8_t GetUsbData(void*     device_ctx,
                   uint16_t* desc_len,
                   char*     descriptors,
                   uint16_t* id_vendor,
                   uint16_t* id_product,
                   char*     manufacturer,
                   char*     product,
                   char*     serial)
{
    DeviceContext* ctx = device_ctx;
    char*          dev_path;
    char           tmp_path[4096];
    char           resolved_link[4096];
    struct stat    sb;
    ssize_t        len;
    char*          rchr;
    int            found = 1;
    FILE*          file;

    if(!ctx) return -1;

    *desc_len   = 0;
    *id_vendor  = 0;
    *id_product = 0;
    memset(tmp_path, 0, 4096);
    memset(resolved_link, 0, 4096);

    if(strncmp(ctx->device_path, "/dev/sd", 7) != 0 && strncmp(ctx->device_path, "/dev/sr", 7) != 0 &&
       strncmp(ctx->device_path, "/dev/st", 7) != 0)
        return 0;

    dev_path = (char*)ctx->device_path + 5;

    snprintf(tmp_path, 4096, "/sys/block/%s", dev_path);

    if(stat(tmp_path, &sb) != 0 || !S_ISDIR(sb.st_mode)) { return 0; }

    len = readlink(tmp_path, resolved_link, 4096);

    if(len == 0) return 0;

    memset(tmp_path, 0, 4096);
    snprintf(tmp_path, 4096, "/sys%s", resolved_link + 2);
    memcpy(resolved_link, tmp_path, 4096);

    while(strstr(resolved_link, "usb") != NULL)
    {
        found = 1;
        rchr  = strrchr(resolved_link, '/');

        if(rchr == NULL) break;

        *rchr = '\0';

        if(strlen(resolved_link) == 0) break;

        snprintf(tmp_path, 4096, "%s/descriptors", resolved_link);
        if(access(tmp_path, R_OK) != 0) found = 0;
        memset(tmp_path, 0, 4096);

        snprintf(tmp_path, 4096, "%s/idProduct", resolved_link);
        if(access(tmp_path, R_OK) != 0) found = 0;
        memset(tmp_path, 0, 4096);

        snprintf(tmp_path, 4096, "%s/idVendor", resolved_link);
        if(access(tmp_path, R_OK) != 0) found = 0;
        memset(tmp_path, 0, 4096);

        if(!found) continue;

        snprintf(tmp_path, 4096, "%s/descriptors", resolved_link);
        file = fopen(tmp_path, "r");

        if(file == NULL) break;

        *desc_len = (uint16_t)fread(descriptors, 4096, 1, file);

        fclose(file);

        memset(tmp_path, 0, 4096);
        snprintf(tmp_path, 4096, "%s/idProduct", resolved_link);

        if(access(tmp_path, R_OK) == 0)
        {
            file = fopen(tmp_path, "r");
            if(file != NULL)
            {
                fscanf(file, "%4hx", id_product);
                fclose(file);
            }
        }

        memset(tmp_path, 0, 4096);
        snprintf(tmp_path, 4096, "%s/idVendor", resolved_link);

        if(access(tmp_path, R_OK) == 0)
        {
            file = fopen(tmp_path, "r");
            if(file != NULL)
            {
                fscanf(file, "%4hx", id_vendor);
                fclose(file);
            }
        }

        memset(tmp_path, 0, 4096);
        snprintf(tmp_path, 4096, "%s/manufacturer", resolved_link);

        if(access(tmp_path, R_OK) == 0)
        {
            file = fopen(tmp_path, "r");
            if(file != NULL)
            {
                fread(manufacturer, 256, 1, file);
                fclose(file);
            }
        }

        memset(tmp_path, 0, 4096);
        snprintf(tmp_path, 4096, "%s/product", resolved_link);

        if(access(tmp_path, R_OK) == 0)
        {
            file = fopen(tmp_path, "r");
            if(file != NULL)
            {
                fread(product, 256, 1, file);
                fclose(file);
            }
        }

        memset(tmp_path, 0, 4096);
        snprintf(tmp_path, 4096, "%s/serial", resolved_link);

        if(access(tmp_path, R_OK) == 0)
        {
            file = fopen(tmp_path, "r");
            if(file != NULL)
            {
                fread(serial, 256, 1, file);
                fclose(file);
            }
        }

        break;
    }

    return *desc_len != 0;
}
