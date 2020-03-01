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

#include "linux.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

uint8_t GetFireWireData(void*     device_ctx,
                        uint32_t* id_model,
                        uint32_t* id_vendor,
                        uint64_t* guid,
                        char*     vendor,
                        char*     model)
{
    DeviceContext* ctx = device_ctx;
    char*          dev_path;
    char           tmp_path[4096];
    char           resolved_link[4096];
    struct stat    sb;
    ssize_t        len;
    char*          rchr;
    int            found;
    FILE*          file;

    if(!ctx) return 0;

    *id_model  = 0;
    *id_vendor = 0;
    *guid      = 0;
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

    while(strstr(resolved_link, "firewire") != NULL)
    {
        found = 1;
        rchr  = strrchr(resolved_link, '/');

        if(rchr == NULL) break;

        *rchr = '\0';

        if(strlen(resolved_link) == 0) break;

        snprintf(tmp_path, 4096, "%s/model", resolved_link);
        if(access(tmp_path, R_OK) != 0) found = 0;
        memset(tmp_path, 0, 4096);

        snprintf(tmp_path, 4096, "%s/vendor", resolved_link);
        if(access(tmp_path, R_OK) != 0) found = 0;
        memset(tmp_path, 0, 4096);

        snprintf(tmp_path, 4096, "%s/guid", resolved_link);
        if(access(tmp_path, R_OK) != 0) found = 0;
        memset(tmp_path, 0, 4096);

        if(!found) continue;

        memset(tmp_path, 0, 4096);
        snprintf(tmp_path, 4096, "%s/model", resolved_link);

        if(access(tmp_path, R_OK) == 0)
        {
            file = fopen(tmp_path, "r");
            if(file != NULL)
            {
                fscanf(file, "%8x", id_model);
                fclose(file);
            }
        }

        memset(tmp_path, 0, 4096);
        snprintf(tmp_path, 4096, "%s/vendor", resolved_link);

        if(access(tmp_path, R_OK) == 0)
        {
            file = fopen(tmp_path, "r");
            if(file != NULL)
            {
                fscanf(file, "%8x", id_vendor);
                fclose(file);
            }
        }

        memset(tmp_path, 0, 4096);
        snprintf(tmp_path, 4096, "%s/guid", resolved_link);

        if(access(tmp_path, R_OK) == 0)
        {
            file = fopen(tmp_path, "r");
            if(file != NULL)
            {
                fscanf(file, "%16lx", guid);
                fclose(file);
            }
        }

        memset(tmp_path, 0, 4096);
        snprintf(tmp_path, 4096, "%s/model_name", resolved_link);

        if(access(tmp_path, R_OK) == 0)
        {
            file = fopen(tmp_path, "r");
            if(file != NULL)
            {
                fread(model, 256, 1, file);
                fclose(file);
            }
        }

        memset(tmp_path, 0, 4096);
        snprintf(tmp_path, 4096, "%s/vendor_name", resolved_link);

        if(access(tmp_path, R_OK) == 0)
        {
            file = fopen(tmp_path, "r");
            if(file != NULL)
            {
                fread(vendor, 256, 1, file);
                fclose(file);
            }
        }

        return 1;
    }

    return 0;
}
