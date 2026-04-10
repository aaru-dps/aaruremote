/*
 * This file is part of the Aaru Remote Server.
 * Copyright (c) 2019-2026 Natalia Portillo.
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

#ifndef AARUREMOTE_MACOS_MACOS_H_
#define AARUREMOTE_MACOS_MACOS_H_

#include <AvailabilityMacros.h>
#include <IOKit/IOBSD.h>
#include <IOKit/IOKitLib.h>
#include <stddef.h>
#include <stdint.h>

typedef struct
{
    int     fd;
    char    device_path[4096];
    char    bsd_name[256];
    int32_t cached_type;
} DeviceContext;

#ifndef __MAC_12_0
#define __MAC_12_0 120000
#endif

#if defined(__MAC_OS_X_VERSION_MAX_ALLOWED) && __MAC_OS_X_VERSION_MAX_ALLOWED >= __MAC_12_0
#define AARUREMOTE_IOKIT_PORT kIOMainPortDefault
#else
#define AARUREMOTE_IOKIT_PORT kIOMasterPortDefault
#endif

void         ExtractBsdName(const char *device_path, char *bsd_name, size_t bsd_name_len);
io_service_t FindDeviceServiceByPath(const char *device_path);
int     CopyRegistryStringProperty(io_registry_entry_t service, const char *dictionary_key, const char *property_key,
                                   char *buffer, size_t buffer_len);
int     CopyRegistryUInt64Property(io_registry_entry_t service, const char *dictionary_key, const char *property_key,
                                   uint64_t *value);
void    PopulateDeviceProperties(io_registry_entry_t service, char *vendor, size_t vendor_len, char *model,
                                 size_t model_len, char *serial, size_t serial_len, char *bus, size_t bus_len);
int     GetBusForService(io_registry_entry_t service, char *buffer, size_t buffer_len);
int     AaruServiceTreeContainsClass(io_registry_entry_t service, const char *class_name_part);
int32_t DetermineDeviceType(io_registry_entry_t service, const char *device_path);

#endif  // AARUREMOTE_MACOS_MACOS_H_
