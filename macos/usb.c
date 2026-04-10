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

#include <string.h>

#include <IOKit/storage/IOStorageDeviceCharacteristics.h>
#include <IOKit/storage/IOStorageProtocolCharacteristics.h>

#include "../aaruremote.h"
#include "macos.h"

uint8_t GetUsbData(void *device_ctx, uint16_t *desc_len, char *descriptors, uint16_t *id_vendor, uint16_t *id_product,
                   char *manufacturer, char *product, char *serial)
{
    DeviceContext *ctx;
    io_service_t   service;
    char           bus[256];
    uint64_t       value;
    int            found;

    ctx = device_ctx;
    if(!ctx) return 0;

    *desc_len   = 0;
    *id_vendor  = 0;
    *id_product = 0;
    memset(descriptors, 0, 4096);
    memset(manufacturer, 0, 256);
    memset(product, 0, 256);
    memset(serial, 0, 256);

    service = FindDeviceServiceByPath(ctx->device_path);
    if(service == IO_OBJECT_NULL) return 0;

    memset(bus, 0, sizeof(bus));
    GetBusForService(service, bus, sizeof(bus));
    if(strncmp(bus, kIOPropertyPhysicalInterconnectTypeUSB, 3) != 0)
    {
        IOObjectRelease(service);
        return 0;
    }

    found = 1;

    if(CopyRegistryUInt64Property(service, NULL, "idVendor", &value) ||
       CopyRegistryUInt64Property(service, NULL, "Vendor ID", &value))
        *id_vendor = (uint16_t)value;

    if(CopyRegistryUInt64Property(service, NULL, "idProduct", &value) ||
       CopyRegistryUInt64Property(service, NULL, "Product ID", &value))
        *id_product = (uint16_t)value;

    if(!CopyRegistryStringProperty(service, kIOPropertyDeviceCharacteristicsKey, kIOPropertyVendorNameKey, manufacturer,
                                   256))
        CopyRegistryStringProperty(service, NULL, "USB Vendor Name", manufacturer, 256);

    if(!CopyRegistryStringProperty(service, kIOPropertyDeviceCharacteristicsKey, kIOPropertyProductNameKey, product,
                                   256))
        CopyRegistryStringProperty(service, NULL, "USB Product Name", product, 256);

    if(!CopyRegistryStringProperty(service, kIOPropertyDeviceCharacteristicsKey, kIOPropertyProductSerialNumberKey,
                                   serial, 256))
        CopyRegistryStringProperty(service, NULL, "USB Serial Number", serial, 256);

    IOObjectRelease(service);
    return found;
}
