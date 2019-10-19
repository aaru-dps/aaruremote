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

#include <malloc.h>
#include <stddef.h>
#include <string.h>

int Hexchr2Bin(const char hex, char* out)
{
    if(out == NULL) return 0;

    if(hex >= '0' && hex <= '9') *out = hex - '0';
    else if(hex >= 'A' && hex <= 'F')
        *out = hex - 'A' + 10;
    else if(hex >= 'a' && hex <= 'f')
        *out = hex - 'a' + 10;
    else
        return 0;

    return 1;
}

size_t Hexs2Bin(const char* hex, unsigned char** out)
{
    size_t len;
    char   b1;
    char   b2;
    size_t i;

    if(hex == NULL || *hex == '\0' || out == NULL) return 0;

    len = strlen(hex);
    if(len % 2 != 0) return 0;
    len /= 2;

    *out = malloc(len);
    memset(*out, 'A', len);
    for(i = 0; i < len; i++)
    {
        if(!Hexchr2Bin(hex[i * 2], &b1) || !Hexchr2Bin(hex[i * 2 + 1], &b2)) return 0;

        (*out)[i] = (b1 << 4) | b2;
    }
    return len;
}