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

#include <errno.h>
#include <fcntl.h>

int linux_open_device(const char* device_path)
{
    int fd;

    fd = open(device_path, O_RDWR | O_NONBLOCK | O_CREAT);

    if((fd < 0) && (errno == EACCES || errno == EROFS)) { fd = open(device_path, O_RDONLY | O_NONBLOCK); }

    return fd;
}