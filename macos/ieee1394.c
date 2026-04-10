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

#include <IOKit/storage/IOFireWireStorageCharacteristics.h>
#include <IOKit/storage/IOStorageDeviceCharacteristics.h>
#include <IOKit/storage/IOStorageProtocolCharacteristics.h>

#include "../aaruremote.h"
#include "macos.h"

uint8_t GetFireWireData(void *device_ctx, uint32_t *id_model, uint32_t *id_vendor, uint64_t *guid, char *vendor,
                        char *model)
{
    DeviceContext *ctx;
    io_service_t   service;
    char           bus[256];
    uint64_t       value;
    int            found;

    ctx = device_ctx;
    if(!ctx) return 0;

    *id_model  = 0;
    *id_vendor = 0;
    *guid      = 0;
    memset(vendor, 0, 256);
    memset(model, 0, 256);

    service = FindDeviceServiceByPath(ctx->device_path);
    if(service == IO_OBJECT_NULL) return 0;

    memset(bus, 0, sizeof(bus));
    GetBusForService(service, bus, sizeof(bus));
    if(strncmp(bus, kIOPropertyPhysicalInterconnectTypeFireWire, 8) != 0)
    {
        IOObjectRelease(service);
        return 0;
    }

    found = 1;

    if(CopyRegistryUInt64Property(service, NULL, "GUID", &value) ||
       CopyRegistryUInt64Property(service, NULL, "FireWire GUID", &value))
        *guid = value;

    if(CopyRegistryUInt64Property(service, NULL, "Model_ID", &value) ||
       CopyRegistryUInt64Property(service, NULL, "Unit_Spec_ID", &value))
        *id_model = (uint32_t)value;

    if(CopyRegistryUInt64Property(service, NULL, "Vendor_ID", &value) ||
       CopyRegistryUInt64Property(service, NULL, "Vendor ID", &value))
        *id_vendor = (uint32_t)value;

    if(!CopyRegistryStringProperty(service, kIOPropertyDeviceCharacteristicsKey, kIOPropertyVendorNameKey, vendor, 256))
        CopyRegistryStringProperty(service, NULL, kIOPropertyBridgeVendorNameKey, vendor, 256);

    if(!CopyRegistryStringProperty(service, kIOPropertyDeviceCharacteristicsKey, kIOPropertyProductNameKey, model, 256))
        CopyRegistryStringProperty(service, NULL, kIOPropertyProductNameKey, model, 256);

    IOObjectRelease(service);
    return found;
}
