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

#ifndef AARUREMOTE_WIN32_USB_H_
#define AARUREMOTE_WIN32_USB_H_

#define USB_BUFFER_SIZE 2048

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

typedef struct _USB_DEVICE_DESCRIPTOR
{
    UCHAR  bLength;
    UCHAR  bDescriptorType;
    USHORT bcdUSB;
    UCHAR  bDeviceClass;
    UCHAR  bDeviceSubClass;
    UCHAR  bDeviceProtocol;
    UCHAR  bMaxPacketSize0;
    USHORT idVendor;
    USHORT idProduct;
    USHORT bcdDevice;
    UCHAR  iManufacturer;
    UCHAR  iProduct;
    UCHAR  iSerialNumber;
    UCHAR  bNumConfigurations;
} USB_DEVICE_DESCRIPTOR, *PUSB_DEVICE_DESCRIPTOR;

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
    DWORD                 PortPortNumber;
    CHAR                  PortHubDevicePath[USB_BUFFER_SIZE];
    DWORD                 PortStatus;
    UCHAR                 PortSpeed;
    BOOL                  PortIsDeviceConnected;
    BOOL                  PortIsHub;
    USB_DEVICE_DESCRIPTOR PortDeviceDescriptor;
} UsbPort_t;

typedef struct _USB_ROOT_HUB_NAME
{
    ULONG ActualLength;
    WCHAR RootHubName[1];
} USB_ROOT_HUB_NAME, *PUSB_ROOT_HUB_NAME;

typedef enum _USB_HUB_NODE
{
    UsbHub,
    UsbMIParent
} USB_HUB_NODE;

typedef struct _USB_HUB_DESCRIPTOR
{
    UCHAR  bDescriptorLength;
    UCHAR  bDescriptorType;
    UCHAR  bNumberOfPorts;
    USHORT wHubCharacteristics;
    UCHAR  bPowerOnToPowerGood;
    UCHAR  bHubControlCurrent;
    UCHAR  bRemoveAndPowerMask[64];
} USB_HUB_DESCRIPTOR, *PUSB_HUB_DESCRIPTOR;

typedef struct _USB_HUB_INFORMATION
{
    USB_HUB_DESCRIPTOR HubDescriptor;
    BOOLEAN            HubIsBusPowered;
} USB_HUB_INFORMATION, *PUSB_HUB_INFORMATION;

typedef struct _USB_MI_PARENT_INFORMATION
{
    ULONG NumberOfInterfaces;
} USB_MI_PARENT_INFORMATION, *PUSB_MI_PARENT_INFORMATION;

typedef struct _USB_NODE_INFORMATION
{
    USB_HUB_NODE NodeType;

    union
    {
        USB_HUB_INFORMATION       HubInformation;
        USB_MI_PARENT_INFORMATION MiParentInformation;
    } u;
} USB_NODE_INFORMATION, *PUSB_NODE_INFORMATION;

#ifndef IOCTL_GET_HCD_DRIVERKEY_NAME
#define IOCTL_GET_HCD_DRIVERKEY_NAME 0x220424
#endif

#ifndef IOCTL_USB_GET_ROOT_HUB_NAME
#define IOCTL_USB_GET_ROOT_HUB_NAME 0x220408
#endif

#ifndef IOCTL_USB_GET_NODE_INFORMATION
#define IOCTL_USB_GET_NODE_INFORMATION 0x220408 // same as above... strange, eh?
#endif

#ifndef IOCTL_USB_GET_NODE_CONNECTION_INFORMATION_EX
#define IOCTL_USB_GET_NODE_CONNECTION_INFORMATION_EX 0x220448
#endif

#ifndef IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION
#define IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION 0x220410
#endif

#ifndef IOCTL_USB_GET_NODE_CONNECTION_NAME
#define IOCTL_USB_GET_NODE_CONNECTION_NAME 0x220414
#endif

#ifndef IOCTL_USB_GET_NODE_CONNECTION_DRIVERKEY_NAME
#define IOCTL_USB_GET_NODE_CONNECTION_DRIVERKEY_NAME 0x220420
#endif

typedef enum _USB_CONNECTION_STATUS
{
    NoDeviceConnected,
    DeviceConnected,
    DeviceFailedEnumeration,
    DeviceGeneralFailure,
    DeviceCausedOvercurrent,
    DeviceNotEnoughPower,
    DeviceNotEnoughBandwidth,
    DeviceHubNestedTooDeeply,
    DeviceInLegacyHub,
    DeviceEnumerating,
    DeviceReset
} USB_CONNECTION_STATUS,
    *PUSB_CONNECTION_STATUS;

typedef struct _USB_ENDPOINT_DESCRIPTOR
{
    UCHAR  bLength;
    UCHAR  bDescriptorType;
    UCHAR  bEndpointAddress;
    UCHAR  bmAttributes;
    USHORT wMaxPacketSize;
    UCHAR  bInterval;
} USB_ENDPOINT_DESCRIPTOR, *PUSB_ENDPOINT_DESCRIPTOR;

typedef struct _USB_PIPE_INFO
{
    USB_ENDPOINT_DESCRIPTOR EndpointDescriptor;
    ULONG                   ScheduleOffset;
} USB_PIPE_INFO, *PUSB_PIPE_INFO;

typedef struct _USB_NODE_CONNECTION_INFORMATION_EX
{
    ULONG                 ConnectionIndex;
    USB_DEVICE_DESCRIPTOR DeviceDescriptor;
    UCHAR                 CurrentConfigurationValue;
    UCHAR                 Speed;
    BOOLEAN               DeviceIsHub;
    USHORT                DeviceAddress;
    ULONG                 NumberOfOpenPipes;
    USB_CONNECTION_STATUS ConnectionStatus;
    USB_PIPE_INFO         PipeList[0];
} USB_NODE_CONNECTION_INFORMATION_EX, *PUSB_NODE_CONNECTION_INFORMATION_EX;

typedef struct _USB_DESCRIPTOR_REQUEST
{
    ULONG ConnectionIndex;

    struct
    {
        UCHAR  bmRequest;
        UCHAR  bRequest;
        USHORT wValue;
        USHORT wIndex;
        USHORT wLength;
    } SetupPacket;

    UCHAR Data[0];
} USB_DESCRIPTOR_REQUEST, *PUSB_DESCRIPTOR_REQUEST;

typedef struct _USB_STRING_DESCRIPTOR
{
    UCHAR bLength;
    UCHAR bDescriptorType;
    WCHAR bString[1];
} USB_STRING_DESCRIPTOR, *PUSB_STRING_DESCRIPTOR;

#ifndef USB_DEVICE_DESCRIPTOR_TYPE
#define USB_DEVICE_DESCRIPTOR_TYPE 0x1
#endif

#ifndef USB_CONFIGURATION_DESCRIPTOR_TYPE
#define USB_CONFIGURATION_DESCRIPTOR_TYPE 0x2
#endif

#ifndef USB_STRING_DESCRIPTOR_TYPE
#define USB_STRING_DESCRIPTOR_TYPE 0x3
#endif

typedef struct _USB_NODE_CONNECTION_NAME
{
    ULONG ConnectionIndex;
    ULONG ActualLength;
    WCHAR NodeName[1];
} USB_NODE_CONNECTION_NAME, *PUSB_NODE_CONNECTION_NAME;

#endif // AARUREMOTE_WIN32_USB_H_
