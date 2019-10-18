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

#include "linux.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

uint8_t linux_get_ieee1394_data(const char* devicePath,
                                uint32_t*   idModel,
                                uint32_t*   idVendor,
                                uint64_t*   guid,
                                char*       vendor,
                                char*       model)
{
    char*       devPath;
    char        tmpPath[4096];
    char        resolvedLink[4096];
    struct stat sb;
    ssize_t     len;
    char*       rchr;
    int         found;
    FILE*       file;

    *idModel  = 0;
    *idVendor = 0;
    *guid     = 0;
    memset(tmpPath, 0, 4096);
    memset(resolvedLink, 0, 4096);

    if(strncmp(devicePath, "/dev/sd", 7) != 0 && strncmp(devicePath, "/dev/sr", 7) != 0 &&
       strncmp(devicePath, "/dev/st", 7) != 0)
        return 0;

    devPath = (char*)devicePath + 5;

    snprintf(tmpPath, 4096, "/sys/block/%s", devPath);

    if(stat(tmpPath, &sb) != 0 || !S_ISDIR(sb.st_mode)) { return 0; }

    len = readlink(tmpPath, resolvedLink, 4096);

    if(len == 0) return 0;

    memset(tmpPath, 0, 4096);
    snprintf(tmpPath, 4096, "/sys%s", resolvedLink + 2);
    memcpy(resolvedLink, tmpPath, 4096);

    while(strstr(resolvedLink, "firewire") != NULL)
    {
        found = 1;
        rchr  = strrchr(resolvedLink, '/');

        if(rchr == NULL) break;

        *rchr = '\0';

        if(strlen(resolvedLink) == 0) break;

        snprintf(tmpPath, 4096, "%s/model", resolvedLink);
        if(access(tmpPath, R_OK) != 0) found = 0;
        memset(tmpPath, 0, 4096);

        snprintf(tmpPath, 4096, "%s/vendor", resolvedLink);
        if(access(tmpPath, R_OK) != 0) found = 0;
        memset(tmpPath, 0, 4096);

        snprintf(tmpPath, 4096, "%s/guid", resolvedLink);
        if(access(tmpPath, R_OK) != 0) found = 0;
        memset(tmpPath, 0, 4096);

        if(!found) continue;

        memset(tmpPath, 0, 4096);
        snprintf(tmpPath, 4096, "%s/model", resolvedLink);

        if(access(tmpPath, R_OK) == 0)
        {
            file = fopen(tmpPath, "r");
            if(file != NULL)
            {
                fscanf(file, "%8x", idModel);
                fclose(file);
            }
        }

        memset(tmpPath, 0, 4096);
        snprintf(tmpPath, 4096, "%s/vendor", resolvedLink);

        if(access(tmpPath, R_OK) == 0)
        {
            file = fopen(tmpPath, "r");
            if(file != NULL)
            {
                fscanf(file, "%8x", idVendor);
                fclose(file);
            }
        }

        memset(tmpPath, 0, 4096);
        snprintf(tmpPath, 4096, "%s/guid", resolvedLink);

        if(access(tmpPath, R_OK) == 0)
        {
            file = fopen(tmpPath, "r");
            if(file != NULL)
            {
                fscanf(file, "%16lx", guid);
                fclose(file);
            }
        }

        memset(tmpPath, 0, 4096);
        snprintf(tmpPath, 4096, "%s/model_name", resolvedLink);

        if(access(tmpPath, R_OK) == 0)
        {
            file = fopen(tmpPath, "r");
            if(file != NULL)
            {
                fread(model, 256, 1, file);
                fclose(file);
            }
        }

        memset(tmpPath, 0, 4096);
        snprintf(tmpPath, 4096, "%s/vendor_name", resolvedLink);

        if(access(tmpPath, R_OK) == 0)
        {
            file = fopen(tmpPath, "r");
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
