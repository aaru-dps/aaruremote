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

#include <CoreFoundation/CoreFoundation.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include <IOKit/storage/IOCDMedia.h>
#include <IOKit/storage/IODVDMedia.h>
#include <IOKit/storage/IOMedia.h>
#include <IOKit/storage/IOStorageDeviceCharacteristics.h>
#include <IOKit/storage/IOStorageProtocolCharacteristics.h>

#include "../aaruremote.h"
#include "macos.h"

static uint32_t AaruGetMilliseconds()
{
    struct timeval tv;

    if(gettimeofday(&tv, NULL) != 0) return 0;

    return (uint32_t)(tv.tv_sec * 1000U + tv.tv_usec / 1000U);
}

void ExtractBsdName(const char *device_path, char *bsd_name, size_t bsd_name_len)
{
    const char *name;

    if(!bsd_name || bsd_name_len == 0) return;

    memset(bsd_name, 0, bsd_name_len);

    if(!device_path) return;

    name = strrchr(device_path, '/');
    if(name)
        name++;
    else
        name = device_path;

    if(name[0] == 'r' && strncmp(name + 1, "disk", 4) == 0) name++;

    strncpy(bsd_name, name, bsd_name_len - 1);
}

static int AaruCFTypeToCString(CFTypeRef value, char *buffer, size_t buffer_len)
{
    CFTypeID type_id;
    CFIndex  data_len;

    if(!value || !buffer || buffer_len == 0) return 0;

    memset(buffer, 0, buffer_len);
    type_id = CFGetTypeID(value);

    if(type_id == CFStringGetTypeID())
        return CFStringGetCString((CFStringRef)value, buffer, buffer_len, kCFStringEncodingUTF8);

    if(type_id == CFDataGetTypeID())
    {
        data_len = CFDataGetLength((CFDataRef)value);

        if(data_len <= 0) return 0;
        if((size_t)data_len >= buffer_len) data_len = (CFIndex)buffer_len - 1;

        memcpy(buffer, CFDataGetBytePtr((CFDataRef)value), (size_t)data_len);
        buffer[data_len] = 0;
        return 1;
    }

    if(type_id == CFNumberGetTypeID())
    {
        long long number_value;

        number_value = 0;
        if(!CFNumberGetValue((CFNumberRef)value, kCFNumberLongLongType, &number_value)) return 0;

        snprintf(buffer, buffer_len, "%lld", number_value);
        return 1;
    }

    return 0;
}

static int AaruCFTypeToUInt64(CFTypeRef value, uint64_t *number)
{
    CFTypeID     type_id;
    char         buffer[128];
    CFIndex      i;
    CFIndex      data_len;
    const UInt8 *bytes;
    uint64_t     converted;

    if(!value || !number) return 0;

    type_id = CFGetTypeID(value);

    if(type_id == CFNumberGetTypeID()) return CFNumberGetValue((CFNumberRef)value, kCFNumberSInt64Type, number);

    if(type_id == CFStringGetTypeID())
    {
        if(!CFStringGetCString((CFStringRef)value, buffer, sizeof(buffer), kCFStringEncodingUTF8)) return 0;

        *number = strtoull(buffer, NULL, 0);
        return 1;
    }

    if(type_id == CFDataGetTypeID())
    {
        bytes     = CFDataGetBytePtr((CFDataRef)value);
        data_len  = CFDataGetLength((CFDataRef)value);
        converted = 0;

        if(data_len <= 0) return 0;
        if(data_len > 8) data_len = 8;

        for(i = 0; i < data_len; i++) converted = (converted << 8) | bytes[i];

        *number = converted;
        return 1;
    }

    return 0;
}

static int AaruCopyPropertyFromDictionary(CFDictionaryRef properties, const char *dictionary_key,
                                          const char *property_key, CFTypeRef *value)
{
    CFStringRef     dict_key_ref;
    CFStringRef     prop_key_ref;
    CFDictionaryRef dictionary;
    CFTypeRef       dict_value;

    if(!properties || !property_key || !value) return 0;

    *value     = NULL;
    dictionary = properties;

    if(dictionary_key)
    {
        dict_key_ref = CFStringCreateWithCString(kCFAllocatorDefault, dictionary_key, kCFStringEncodingUTF8);
        if(!dict_key_ref) return 0;

        dict_value = CFDictionaryGetValue(properties, dict_key_ref);
        CFRelease(dict_key_ref);

        if(!dict_value || CFGetTypeID(dict_value) != CFDictionaryGetTypeID()) return 0;

        dictionary = (CFDictionaryRef)dict_value;
    }

    prop_key_ref = CFStringCreateWithCString(kCFAllocatorDefault, property_key, kCFStringEncodingUTF8);
    if(!prop_key_ref) return 0;

    *value = CFDictionaryGetValue(dictionary, prop_key_ref);
    CFRelease(prop_key_ref);

    return *value != NULL;
}

int CopyRegistryStringProperty(io_registry_entry_t service, const char *dictionary_key, const char *property_key,
                               char *buffer, size_t buffer_len)
{
    io_registry_entry_t    current;
    io_registry_entry_t    parent;
    CFMutableDictionaryRef properties;
    CFTypeRef              value;
    int                    found;

    if(!buffer || buffer_len == 0) return 0;

    memset(buffer, 0, buffer_len);
    if(service == IO_OBJECT_NULL) return 0;

    current = service;
    found   = 0;

    while(current != IO_OBJECT_NULL)
    {
        properties = NULL;
        if(IORegistryEntryCreateCFProperties(current, &properties, kCFAllocatorDefault, kNilOptions) == KERN_SUCCESS &&
           properties != NULL)
        {
            if(AaruCopyPropertyFromDictionary(properties, dictionary_key, property_key, &value))
                found = AaruCFTypeToCString(value, buffer, buffer_len);

            CFRelease(properties);
        }

        if(found) break;

        parent = IO_OBJECT_NULL;
        if(IORegistryEntryGetParentEntry(current, kIOServicePlane, &parent) != KERN_SUCCESS) break;

        if(current != service) IOObjectRelease(current);
        current = parent;
    }

    if(current != IO_OBJECT_NULL && current != service) IOObjectRelease(current);

    return found;
}

int CopyRegistryUInt64Property(io_registry_entry_t service, const char *dictionary_key, const char *property_key,
                               uint64_t *value)
{
    io_registry_entry_t    current;
    io_registry_entry_t    parent;
    CFMutableDictionaryRef properties;
    CFTypeRef              property_value;
    int                    found;

    if(!value) return 0;

    *value = 0;
    if(service == IO_OBJECT_NULL) return 0;

    current = service;
    found   = 0;

    while(current != IO_OBJECT_NULL)
    {
        properties = NULL;
        if(IORegistryEntryCreateCFProperties(current, &properties, kCFAllocatorDefault, kNilOptions) == KERN_SUCCESS &&
           properties != NULL)
        {
            if(AaruCopyPropertyFromDictionary(properties, dictionary_key, property_key, &property_value))
                found = AaruCFTypeToUInt64(property_value, value);

            CFRelease(properties);
        }

        if(found) break;

        parent = IO_OBJECT_NULL;
        if(IORegistryEntryGetParentEntry(current, kIOServicePlane, &parent) != KERN_SUCCESS) break;

        if(current != service) IOObjectRelease(current);
        current = parent;
    }

    if(current != IO_OBJECT_NULL && current != service) IOObjectRelease(current);

    return found;
}

void PopulateDeviceProperties(io_registry_entry_t service, char *vendor, size_t vendor_len, char *model,
                              size_t model_len, char *serial, size_t serial_len, char *bus, size_t bus_len)
{
    io_registry_entry_t    current;
    io_registry_entry_t    parent;
    CFMutableDictionaryRef properties;
    CFTypeRef              value;
    int                    have_vendor;
    int                    have_model;
    int                    have_serial;
    int                    have_bus;

    if(vendor && vendor_len > 0) memset(vendor, 0, vendor_len);
    if(model && model_len > 0) memset(model, 0, model_len);
    if(serial && serial_len > 0) memset(serial, 0, serial_len);
    if(bus && bus_len > 0) memset(bus, 0, bus_len);

    if(service == IO_OBJECT_NULL) return;

    current     = service;
    have_vendor = vendor == NULL || vendor_len == 0;
    have_model  = model == NULL || model_len == 0;
    have_serial = serial == NULL || serial_len == 0;
    have_bus    = bus == NULL || bus_len == 0;

    while(current != IO_OBJECT_NULL && !(have_vendor && have_model && have_serial && have_bus))
    {
        properties = NULL;
        if(IORegistryEntryCreateCFProperties(current, &properties, kCFAllocatorDefault, kNilOptions) == KERN_SUCCESS &&
           properties != NULL)
        {
            if(!have_vendor && (AaruCopyPropertyFromDictionary(properties, kIOPropertyDeviceCharacteristicsKey,
                                                               kIOPropertyVendorNameKey, &value) ||
                                AaruCopyPropertyFromDictionary(properties, NULL, kIOPropertyVendorNameKey, &value)))
                have_vendor = AaruCFTypeToCString(value, vendor, vendor_len);

            if(!have_model && (AaruCopyPropertyFromDictionary(properties, kIOPropertyDeviceCharacteristicsKey,
                                                              kIOPropertyProductNameKey, &value) ||
                               AaruCopyPropertyFromDictionary(properties, NULL, kIOPropertyProductNameKey, &value)))
                have_model = AaruCFTypeToCString(value, model, model_len);

            if(!have_serial &&
               (AaruCopyPropertyFromDictionary(properties, kIOPropertyDeviceCharacteristicsKey,
                                               kIOPropertyProductSerialNumberKey, &value) ||
                AaruCopyPropertyFromDictionary(properties, NULL, kIOPropertyProductSerialNumberKey, &value)))
                have_serial = AaruCFTypeToCString(value, serial, serial_len);

            if(!have_bus &&
               (AaruCopyPropertyFromDictionary(properties, kIOPropertyProtocolCharacteristicsKey,
                                               kIOPropertyPhysicalInterconnectTypeKey, &value) ||
                AaruCopyPropertyFromDictionary(properties, NULL, kIOPropertyPhysicalInterconnectTypeKey, &value)))
                have_bus = AaruCFTypeToCString(value, bus, bus_len);

            CFRelease(properties);
        }

        if(have_vendor && have_model && have_serial && have_bus) break;

        parent = IO_OBJECT_NULL;
        if(IORegistryEntryGetParentEntry(current, kIOServicePlane, &parent) != KERN_SUCCESS) break;

        if(current != service) IOObjectRelease(current);
        current = parent;
    }

    if(current != IO_OBJECT_NULL && current != service) IOObjectRelease(current);
}

int GetBusForService(io_registry_entry_t service, char *buffer, size_t buffer_len)
{
    PopulateDeviceProperties(service, NULL, 0, NULL, 0, NULL, 0, buffer, buffer_len);

    return buffer != NULL && buffer[0] != 0;
}

int AaruServiceTreeContainsClass(io_registry_entry_t service, const char *class_name_part)
{
    io_registry_entry_t current;
    io_registry_entry_t parent;
    io_name_t           class_name;

    if(service == IO_OBJECT_NULL || !class_name_part) return 0;

    current = service;

    while(current != IO_OBJECT_NULL)
    {
        memset(class_name, 0, sizeof(class_name));
        if(IOObjectGetClass(current, class_name) == KERN_SUCCESS && strstr(class_name, class_name_part) != NULL)
        {
            if(current != service) IOObjectRelease(current);
            return 1;
        }

        parent = IO_OBJECT_NULL;
        if(IORegistryEntryGetParentEntry(current, kIOServicePlane, &parent) != KERN_SUCCESS) break;

        if(current != service) IOObjectRelease(current);
        current = parent;
    }

    if(current != IO_OBJECT_NULL && current != service) IOObjectRelease(current);

    return 0;
}

int32_t DetermineDeviceType(io_registry_entry_t service, const char *device_path)
{
    char bus[256];
    int  is_optical;

    if(service == IO_OBJECT_NULL) return AARUREMOTE_DEVICE_TYPE_UNKNOWN;

    memset(bus, 0, sizeof(bus));
    GetBusForService(service, bus, sizeof(bus));

    is_optical = IOObjectConformsTo(service, kIOCDMediaClass) || IOObjectConformsTo(service, kIODVDMediaClass);

    if(strncmp(bus, kIOPropertyPhysicalInterconnectTypeATAPI, 5) == 0) return AARUREMOTE_DEVICE_TYPE_ATAPI;

    if(strncmp(bus, kIOPropertyPhysicalInterconnectTypeATA, 3) == 0 ||
       strncmp(bus, kIOPropertyPhysicalInterconnectTypeSerialATA, 4) == 0)
        return is_optical ? AARUREMOTE_DEVICE_TYPE_ATAPI : AARUREMOTE_DEVICE_TYPE_ATA;

    if(strncmp(bus, kIOPropertyPhysicalInterconnectTypeSecureDigital, 14) == 0)
        return AARUREMOTE_DEVICE_TYPE_SECURE_DIGITAL;

    if(strncmp(bus, kIOPropertyPhysicalInterconnectTypePCIExpress, 11) == 0 ||
       strncmp(bus, kIOPropertyPhysicalInterconnectTypeAppleFabric, 12) == 0 ||
       AaruServiceTreeContainsClass(service, "NVMe") || (device_path && strstr(device_path, "nvme") != NULL))
        return AARUREMOTE_DEVICE_TYPE_NVME;

    if(strncmp(bus, kIOPropertyPhysicalInterconnectTypeUSB, 3) == 0 ||
       strncmp(bus, kIOPropertyPhysicalInterconnectTypeFireWire, 8) == 0 ||
       strncmp(bus, kIOPropertyPhysicalInterconnectTypeSCSIParallel, 5) == 0 ||
       strncmp(bus, kIOPropertyPhysicalInterconnectTypeSerialAttachedSCSI, 3) == 0 ||
       strncmp(bus, kIOPropertyPhysicalInterconnectTypeFibreChannel, 5) == 0)
        return AARUREMOTE_DEVICE_TYPE_SCSI;

    if(bus[0] != 0 && strncmp(bus, kIOPropertyPhysicalInterconnectTypeVirtual, 7) == 0)
        return AARUREMOTE_DEVICE_TYPE_UNKNOWN;

    return AARUREMOTE_DEVICE_TYPE_SCSI;
}

io_service_t FindDeviceServiceByPath(const char *device_path)
{
    char                   bsd_name[256];
    CFMutableDictionaryRef matching;

    ExtractBsdName(device_path, bsd_name, sizeof(bsd_name));
    if(bsd_name[0] == 0) return IO_OBJECT_NULL;

    matching = IOBSDNameMatching(AARUREMOTE_IOKIT_PORT, 0, bsd_name);
    if(!matching) return IO_OBJECT_NULL;

    return IOServiceGetMatchingService(AARUREMOTE_IOKIT_PORT, matching);
}

void *DeviceOpen(const char *device_path)
{
    DeviceContext *ctx;
    char          *real_device_path;

    real_device_path = realpath(device_path, NULL);
    if(real_device_path != NULL) device_path = real_device_path;

    ctx = malloc(sizeof(DeviceContext));
    if(!ctx)
    {
        free(real_device_path);
        return NULL;
    }

    memset(ctx, 0, sizeof(DeviceContext));
    ctx->cached_type = AARUREMOTE_DEVICE_TYPE_UNKNOWN;

    ctx->fd = open(device_path, O_RDWR | O_NONBLOCK);
    if((ctx->fd < 0) && (errno == EACCES || errno == EROFS)) ctx->fd = open(device_path, O_RDONLY | O_NONBLOCK);

    if(ctx->fd < 0)
    {
        free(real_device_path);
        free(ctx);
        return NULL;
    }

    strncpy(ctx->device_path, device_path, sizeof(ctx->device_path) - 1);
    ExtractBsdName(device_path, ctx->bsd_name, sizeof(ctx->bsd_name));

    free(real_device_path);
    return ctx;
}

void DeviceClose(void *device_ctx)
{
    DeviceContext *ctx = device_ctx;

    if(!ctx) return;

    if(ctx->fd >= 0) close(ctx->fd);

    free(ctx);
}

int32_t GetDeviceType(void *device_ctx)
{
    DeviceContext *ctx;
    io_service_t   service;

    ctx = device_ctx;
    if(!ctx) return -1;

    if(ctx->cached_type != AARUREMOTE_DEVICE_TYPE_UNKNOWN) return ctx->cached_type;

    service = FindDeviceServiceByPath(ctx->device_path);
    if(service == IO_OBJECT_NULL) return AARUREMOTE_DEVICE_TYPE_UNKNOWN;

    ctx->cached_type = DetermineDeviceType(service, ctx->device_path);
    IOObjectRelease(service);

    return ctx->cached_type;
}

int32_t ReOpen(void *device_ctx, uint32_t *closeFailed)
{
    DeviceContext *ctx;
    int            ret;

    ctx = device_ctx;
    if(closeFailed) *closeFailed = 0;

    if(!ctx) return -1;

    ret = close(ctx->fd);
    if(ret < 0)
    {
        if(closeFailed) *closeFailed = 1;
        return errno;
    }

    ctx->fd = open(ctx->device_path, O_RDWR | O_NONBLOCK);
    if((ctx->fd < 0) && (errno == EACCES || errno == EROFS)) ctx->fd = open(ctx->device_path, O_RDONLY | O_NONBLOCK);

    return ctx->fd < 0 ? errno : 0;
}

int32_t OsRead(void *device_ctx, char *buffer, uint64_t offset, uint32_t length, uint32_t *duration)
{
    DeviceContext *ctx;
    ssize_t        ret;
    uint32_t       start_ms;
    uint32_t       end_ms;

    ctx = device_ctx;
    if(duration) *duration = 0;

    if(!ctx) return -1;

    start_ms = AaruGetMilliseconds();
    ret      = pread(ctx->fd, buffer, (size_t)length, (off_t)offset);
    end_ms   = AaruGetMilliseconds();

    if(duration && end_ms >= start_ms) *duration = end_ms - start_ms;

    return ret < 0 ? errno : 0;
}
