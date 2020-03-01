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

#include <dirent.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

uint8_t GetPcmciaData(void* device_ctx, uint16_t* cis_len, char* cis)
{
    DeviceContext* ctx = device_ctx;
    char*          dev_path;
    char           tmp_path[4096];
    char           resolved_link[4096];
    struct stat    sb;
    ssize_t        len;
    char*          rchr;
    FILE*          file;
    DIR*           dir;
    struct dirent* dent;
    *cis_len = 0;

    if(!ctx) return 0;

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

    while(strstr(resolved_link, "/sys/devices") != NULL)
    {
        rchr = strrchr(resolved_link, '/');

        if(rchr == NULL) break;

        *rchr = '\0';

        if(strlen(resolved_link) == 0) break;

        memset(tmp_path, 0, 4096);
        snprintf(tmp_path, 4096, "%s/pcmcia_socket", resolved_link);
        if(access(tmp_path, R_OK) != 0) continue;

        dir = opendir(tmp_path);
        if(!dir) continue;

        do
        {
            dent = readdir(dir);

            if(!dent) break;

        } while(dent && dent->d_type != DT_DIR);

        if(!dent) continue;

        memset(tmp_path, 0, 4096);
        snprintf(tmp_path, 4096, "%s/pcmcia_socket/%s/cis", resolved_link, dent->d_name);
        if(access(tmp_path, R_OK) != 0) continue;

        file = fopen(tmp_path, "r");
        if(!file) return 0;

        *cis_len = fread(cis, 65536, 1, file);

        return 1;
    }

    return 0;
}
