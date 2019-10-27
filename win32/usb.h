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

#define USB_BUFFER_SIZE 2048

#include <WinSock2.h>
#include <usbioctl.h>
#include <windows.h>

#ifndef GUID_DEVINTERFACE_USB_HOST_CONTROLLER
#define GUID_DEVINTERFACE_USB_HOST_CONTROLLER                                                                          \
    {                                                                                                                  \
        0x3abf6f2d, 0x71c4, 0x462a, { 0x8a, 0x92, 0x1e, 0x68, 0x61, 0xe6, 0xaf, 0x27 }                                 \
    }
#endif

typedef struct
{
    DWORD ControllerIndex;
    CHAR  ControllerDevicePath[USB_BUFFER_SIZE];
    CHAR  ControllerDeviceDesc[USB_BUFFER_SIZE];
    CHAR  ControllerDriverKeyName[USB_BUFFER_SIZE];
} UsbController_t;

typedef struct
{
    CHAR                  InstanceId[USB_BUFFER_SIZE];
    CHAR                  Manufacturer[USB_BUFFER_SIZE];
    CHAR                  Product[USB_BUFFER_SIZE];
    CHAR                  SerialNumber[USB_BUFFER_SIZE];
    CHAR                  DriverKey[USB_BUFFER_SIZE];
    DWORD                 PortNumber;
    USB_DEVICE_DESCRIPTOR DeviceDescriptor;
    CHAR                  HubDevicePath[USB_BUFFER_SIZE];
    CHAR                  BinaryDeviceDescriptors[USB_BUFFER_SIZE];
    DWORD                 BinaryDeviceDescriptorsLength;
} UsbDevice_t;

typedef struct
{
    BOOL  HubIsRootHub;
    CHAR  HubDeviceDesc[USB_BUFFER_SIZE];
    CHAR  HubDevicePath[USB_BUFFER_SIZE];
    BOOL  HubIsBusPowered;
    UCHAR HubPortCount;
    CHAR  InstanceId[USB_BUFFER_SIZE];
    CHAR  Manufacturer[USB_BUFFER_SIZE];
    CHAR  Product[USB_BUFFER_SIZE];
    CHAR  SerialNumber[USB_BUFFER_SIZE];
    CHAR  DriverKey[USB_BUFFER_SIZE];
} UsbHub_t;

typedef struct
{
    DWORD                 PortPortNumber;
    CHAR                  PortHubDevicePath[USB_BUFFER_SIZE];
    DWORD                 PortStatus;
    UCHAR                 PortSpeed;
    BOOL                  PortIsDeviceConnected;
    BOOL                  PortIsHub;
    USB_DEVICE_DESCRIPTOR PortDeviceDescriptor;
} UsbPort_t;