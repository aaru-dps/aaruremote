/*
 * This file is part of the Aaru Remote Server.
 * Copyright (c) 2019-2021 Natalia Portillo.
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

#include <windows.h>

#include <SetupAPI.h>
#include <cfgmgr32.h>
#include <guiddef.h>
#include <stdio.h>

#include "win32.h"

#include "usb.h"

#include "ntioctl.h"

#ifndef DIGCF_PRESENT
#define DIGCF_PRESENT 0x00000002
#endif
#ifndef DIGCF_DEVICEINTERFACE
#define DIGCF_DEVICEINTERFACE 0x00000010
#endif

UsbController_t* GetHostControllers(DWORD* length)
{
    GUID                             hostGuid = GUID_DEVINTERFACE_USB_HOST_CONTROLLER;
    HDEVINFO                         h;
    UsbController_t*                 hostlist;
    UsbController_t*                 host;
    DWORD                            i;
    BOOL                             success;
    SP_DEVICE_INTERFACE_DATA         dia;
    SP_DEVINFO_DATA                  da;
    PSP_DEVICE_INTERFACE_DETAIL_DATA didd;
    DWORD                            nRequiredSize = 0;
    DWORD                            N_BYTES       = USB_BUFFER_SIZE;
    DWORD                            requiredSize;
    DWORD                            regType;
    CHAR                             ptrBuf[USB_BUFFER_SIZE];

    *length = 0;

    didd = malloc(sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA) + USB_BUFFER_SIZE);

    hostlist = malloc(sizeof(UsbController_t) * 64);
    if(!hostlist) return NULL;

    memset(hostlist, 0, sizeof(UsbController_t) * 64);

    // We start at the "root" of the device tree and look for all
    // devices that match the interface GUID of a Hub Controller
    h = SetupDiGetClassDevs(&hostGuid, 0, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

    if(h == INVALID_HANDLE_VALUE) return NULL;

    i = 0;

    do {
        host = &hostlist[*length];
        memset(host, 0, sizeof(UsbController_t));
        host->ControllerIndex = i;

        // create a Device Interface Data structure
        memset(&dia, 0, sizeof(SP_DEVICE_INTERFACE_DATA));
        dia.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

        // start the enumeration
        success = SetupDiEnumDeviceInterfaces(h, NULL, &hostGuid, i, &dia);
        if(success)
        {
            // build a DevInfo Data structure
            memset(&da, 0, sizeof(SP_DEVINFO_DATA));
            da.cbSize = sizeof(SP_DEVINFO_DATA);
            // build a Device Interface Detail Data structure
            memset(didd, 0, USB_BUFFER_SIZE + sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA));
            didd->cbSize = 4 + sizeof(TCHAR);

            // now we can get some more detailed information
            nRequiredSize = 0;
            if(SetupDiGetDeviceInterfaceDetail(h, &dia, didd, N_BYTES, &nRequiredSize, &da))
            {
                strncpy(host->ControllerDevicePath, didd->DevicePath, USB_BUFFER_SIZE);

                // get the Device Description and DriverKeyName
                requiredSize = 0;
                regType      = REG_SZ;

                if(SetupDiGetDeviceRegistryProperty(
                       h, &da, SPDRP_DEVICEDESC, &regType, (PBYTE)ptrBuf, USB_BUFFER_SIZE, &requiredSize))
                    strncpy(host->ControllerDeviceDesc, ptrBuf, USB_BUFFER_SIZE);
                if(SetupDiGetDeviceRegistryProperty(
                       h, &da, SPDRP_DRIVER, &regType, (PBYTE)ptrBuf, USB_BUFFER_SIZE, &requiredSize))
                    strncpy(host->ControllerDriverKeyName, ptrBuf, USB_BUFFER_SIZE);
            }

            (*length)++;
        }

        i++;
    } while(success);

    SetupDiDestroyDeviceInfoList(h);

    return hostlist;
}

DWORD GetDeviceNumberWithHandle(HANDLE handle)
{
    STORAGE_DEVICE_NUMBER sdn;
    DWORD                 ans = -1;
    DWORD                 k   = 0;

    if(DeviceIoControl(handle, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0, &sdn, sizeof(STORAGE_DEVICE_NUMBER), &k, NULL))
        ans = (sdn.DeviceType << 8) + sdn.DeviceNumber;

    return ans;
}

DWORD GetDeviceNumber(char* devicePath)
{
    DWORD  len = (DWORD)strlen(devicePath);
    HANDLE h;
    DWORD  ans = -1;

    if(devicePath[len - 1] == '\\') devicePath[len - 1] = 0;

    h = CreateFile(devicePath, 0, 0, NULL, OPEN_EXISTING, 0, NULL);

    if(h != INVALID_HANDLE_VALUE)
    {
        ans = GetDeviceNumberWithHandle(h);
        CloseHandle(h);
    }

    return ans;
}

UsbHub_t* GetRootHub(UsbController_t* controller)
{
    HANDLE               h, h2;
    UsbHub_t*            root;
    PUSB_ROOT_HUB_NAME   hubName;
    DWORD                nBytes;
    DWORD                k = 0;
    USB_NODE_INFORMATION nodeInfo;

    root    = malloc(sizeof(UsbHub_t));
    hubName = malloc(sizeof(USB_ROOT_HUB_NAME) + USB_BUFFER_SIZE - 1);

    if(!root) return NULL;

    root->HubIsRootHub = TRUE;
    strncpy(root->HubDeviceDesc, "Root Hub", 8);

    // Open a handle to the Host Controller
    h = CreateFile(controller->ControllerDevicePath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

    if(h == INVALID_HANDLE_VALUE) return root;

    memset(hubName, 0, sizeof(USB_ROOT_HUB_NAME) + USB_BUFFER_SIZE - 1);
    nBytes = sizeof(USB_ROOT_HUB_NAME) + USB_BUFFER_SIZE - 1;

    // get the Hub Name
    if(DeviceIoControl(h, IOCTL_USB_GET_ROOT_HUB_NAME, &hubName, nBytes, &hubName, nBytes, &k, NULL))
        sprintf_s(root->HubDevicePath, USB_BUFFER_SIZE, "\\\\.\\%ws", hubName->RootHubName);

    // TODO: Get DriverKeyName for Root Hub

    // Now let's open the Hub (based upon the HubName we got above)
    h2 = CreateFile(root->HubDevicePath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if(h2 != INVALID_HANDLE_VALUE)
    {
        memset(&nodeInfo, 0, sizeof(USB_NODE_INFORMATION));
        nodeInfo.NodeType = UsbHub;

        nBytes = sizeof(USB_NODE_INFORMATION);

        // get the Hub Information
        if(DeviceIoControl(h2, IOCTL_USB_GET_NODE_INFORMATION, &nodeInfo, nBytes, &nodeInfo, nBytes, &k, NULL))
        {
            root->HubIsBusPowered = nodeInfo.u.HubInformation.HubIsBusPowered;
            root->HubPortCount    = nodeInfo.u.HubInformation.HubDescriptor.bNumberOfPorts;
        }

        CloseHandle(h2);
    }

    free(hubName);
    CloseHandle(h);
    return root;
}

UsbPort_t* GetHubPorts(UsbHub_t* hub, DWORD* length)
{
    UsbPort_t*                         portList;
    UsbPort_t*                         port;
    HANDLE                             h;
    USB_NODE_CONNECTION_INFORMATION_EX nodeConnection;
    DWORD                              nBytes;
    DWORD                              k = 0;
    DWORD                              i;

    *length = 0;

    if(hub == NULL) return NULL;

    h = CreateFile(hub->HubDevicePath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

    if(h == INVALID_HANDLE_VALUE) return NULL;

    portList = malloc(sizeof(UsbPort_t) * hub->HubPortCount);

    if(!portList) return NULL;

    for(i = 1; i <= hub->HubPortCount; i++)
    {
        nBytes = sizeof(USB_NODE_CONNECTION_INFORMATION_EX);
        memset(&nodeConnection, 0, nBytes);
        nodeConnection.ConnectionIndex = i;
        k                              = 0;

        if(!DeviceIoControl(h,
                            IOCTL_USB_GET_NODE_CONNECTION_INFORMATION_EX,
                            &nodeConnection,
                            nBytes,
                            &nodeConnection,
                            nBytes,
                            &k,
                            NULL))
            continue;

        // load up the USBPort class
        port = &portList[*length];

        port->PortPortNumber = i;
        strncpy(port->PortHubDevicePath, hub->HubDevicePath, USB_BUFFER_SIZE);
        port->PortStatus            = nodeConnection.ConnectionStatus;
        port->PortSpeed             = nodeConnection.Speed;
        port->PortIsDeviceConnected = nodeConnection.ConnectionStatus == DeviceConnected;
        port->PortIsHub             = nodeConnection.DeviceIsHub;
        memcpy(&port->PortDeviceDescriptor, &nodeConnection.DeviceDescriptor, sizeof(USB_DEVICE_DESCRIPTOR));

        // add it to the list
        (*length)++;
    }

    CloseHandle(h);
    return portList;
}

UsbDevice_t* GetPortDevice(UsbPort_t* port)
{
    UsbDevice_t*            device;
    HANDLE                  h;
    DWORD                   nBytesReturned;
    DWORD                   nBytes = USB_BUFFER_SIZE + sizeof(USB_DESCRIPTOR_REQUEST) + sizeof(USB_STRING_DESCRIPTOR);
    PUSB_DESCRIPTOR_REQUEST request;
    PUSB_STRING_DESCRIPTOR  descriptor;
    DWORD                   k;

    if(port == NULL || port->PortIsDeviceConnected) return NULL;

    device  = malloc(sizeof(UsbDevice_t));
    request = malloc(nBytes);

    if(!device) return NULL;

    memset(device, 0, sizeof(UsbDevice_t));

    // Copy over some values from the Port class
    device->PortNumber = port->PortPortNumber;
    strncpy(device->HubDevicePath, port->PortHubDevicePath, USB_BUFFER_SIZE);
    memcpy(&device->DeviceDescriptor, &port->PortDeviceDescriptor, sizeof(USB_DEVICE_DESCRIPTOR));

    // Open a handle to the Hub device
    h = CreateFile(port->PortHubDevicePath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if(h == INVALID_HANDLE_VALUE) return device;

    // The iManufacturer, iProduct and iSerialNumber entries in the
    // Device Descriptor are really just indexes.  So, we have to
    // request a String Descriptor to get the values for those strings.
    if(port->PortDeviceDescriptor.iManufacturer > 0)
    {
        memset(request, 0, nBytes);

        request->ConnectionIndex     = port->PortPortNumber;
        request->SetupPacket.wIndex  = 0x409;
        request->SetupPacket.wValue  = ((USB_STRING_DESCRIPTOR_TYPE << 8) + port->PortDeviceDescriptor.iManufacturer);
        request->SetupPacket.wLength = (USHORT)(nBytes - sizeof(USB_DESCRIPTOR_REQUEST));

        // Use an IOCTL call to request the String Descriptor
        if(DeviceIoControl(
               h, IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION, request, nBytes, request, nBytes, &k, NULL))
        {
            // The location of the string descriptor is immediately after
            // the Request structure.  Because this location is not "covered"
            // by the structure allocation, we're forced to zero out this
            // chunk of memory by using the StringToHGlobalAuto() hack above
            descriptor = (PUSB_STRING_DESCRIPTOR)(request + sizeof(USB_DESCRIPTOR_REQUEST));
            sprintf_s(device->Manufacturer, USB_BUFFER_SIZE, "%ws", descriptor->bString);
        }
    }
    if(port->PortDeviceDescriptor.iProduct > 0)
    {
        memset(request, 0, nBytes);

        request->ConnectionIndex     = port->PortPortNumber;
        request->SetupPacket.wIndex  = 0x409;
        request->SetupPacket.wValue  = ((USB_STRING_DESCRIPTOR_TYPE << 8) + port->PortDeviceDescriptor.iProduct);
        request->SetupPacket.wLength = (USHORT)(nBytes - sizeof(USB_DESCRIPTOR_REQUEST));

        // Use an IOCTL call to request the String Descriptor
        if(DeviceIoControl(
               h, IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION, request, nBytes, request, nBytes, &k, NULL))
        {
            // The location of the string descriptor is immediately after
            // the Request structure.  Because this location is not "covered"
            // by the structure allocation, we're forced to zero out this
            // chunk of memory by using the StringToHGlobalAuto() hack above
            descriptor = (PUSB_STRING_DESCRIPTOR)(request + sizeof(USB_DESCRIPTOR_REQUEST));
            sprintf_s(device->Product, USB_BUFFER_SIZE, "%ws", descriptor->bString);
        }
    }
    if(port->PortDeviceDescriptor.iSerialNumber > 0)
    {
        memset(request, 0, nBytes);

        request->ConnectionIndex     = port->PortPortNumber;
        request->SetupPacket.wIndex  = 0x409;
        request->SetupPacket.wValue  = ((USB_STRING_DESCRIPTOR_TYPE << 8) + port->PortDeviceDescriptor.iSerialNumber);
        request->SetupPacket.wLength = (USHORT)(nBytes - sizeof(USB_DESCRIPTOR_REQUEST));

        // Use an IOCTL call to request the String Descriptor
        if(DeviceIoControl(
               h, IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION, request, nBytes, request, nBytes, &k, NULL))
        {
            // The location of the string descriptor is immediately after
            // the Request structure.  Because this location is not "covered"
            // by the structure allocation, we're forced to zero out this
            // chunk of memory by using the StringToHGlobalAuto() hack above
            descriptor = (PUSB_STRING_DESCRIPTOR)(request + sizeof(USB_DESCRIPTOR_REQUEST));
            sprintf_s(device->SerialNumber, USB_BUFFER_SIZE, "%ws", descriptor->bString);
        }
    }

    // build a request for configuration descriptor
    memset(request, 0, nBytes);

    request->ConnectionIndex     = port->PortPortNumber;
    request->SetupPacket.wIndex  = 0;
    request->SetupPacket.wValue  = ((USB_STRING_DESCRIPTOR_TYPE << 8));
    request->SetupPacket.wLength = (USHORT)(nBytes - sizeof(USB_DESCRIPTOR_REQUEST));

    // Use an IOCTL call to request the String Descriptor
    if(DeviceIoControl(
           h, IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION, request, nBytes, request, nBytes, &nBytesReturned, NULL))
    {
        if(nBytesReturned > USB_BUFFER_SIZE) nBytesReturned = USB_BUFFER_SIZE;

        memcpy(&device->BinaryDeviceDescriptors, request + sizeof(USB_DESCRIPTOR_REQUEST), nBytesReturned);
        device->BinaryDeviceDescriptorsLength = nBytesReturned;
    }

    free(request);
    CloseHandle(h);
    return device;
}

UsbHub_t* GetPortHub(UsbPort_t* port)
{
    UsbHub_t*                 hub;
    HANDLE                    h, h2;
    PUSB_NODE_CONNECTION_NAME nodeName;
    DWORD                     nBytes;
    DWORD                     k = 0;
    USB_NODE_INFORMATION      nodeInfo;
    UsbDevice_t*              device;

    nodeName = malloc(sizeof(USB_NODE_CONNECTION_NAME) + USB_BUFFER_SIZE - 1);

    if(port == NULL || port->PortIsHub || nodeName == NULL) return NULL;

    hub = malloc(sizeof(UsbHub_t));

    if(!hub) return NULL;

    hub->HubIsRootHub = FALSE;
    strncpy(hub->HubDeviceDesc, "External Hub", 12);

    // Open a handle to the Host Controller
    h = CreateFile(port->PortHubDevicePath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if(h == INVALID_HANDLE_VALUE)
    {
        free(nodeName);
        return hub;
    }

    // Get the DevicePath for downstream hub
    nBytes = sizeof(USB_NODE_CONNECTION_NAME) + USB_BUFFER_SIZE - 1;
    memset(nodeName, 0, nBytes);
    nodeName->ConnectionIndex = port->PortPortNumber;
    k                         = 0;

    // Use an IOCTL call to request the Node Name
    if(DeviceIoControl(h, IOCTL_USB_GET_NODE_CONNECTION_NAME, nodeName, nBytes, nodeName, nBytes, &k, NULL))
        sprintf_s(hub->HubDevicePath, USB_BUFFER_SIZE, "\\\\.\\%ws", nodeName->NodeName);

    // Now let's open the Hub (based upon the HubName we got above)
    h2 = CreateFile(hub->HubDevicePath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

    if(h2 != INVALID_HANDLE_VALUE)
    {
        memset(&nodeInfo, 0, sizeof(USB_NODE_INFORMATION));
        nodeInfo.NodeType = UsbHub;
        nBytes            = sizeof(USB_NODE_INFORMATION);
        k                 = 0;

        // get the Hub Information
        if(DeviceIoControl(h2, IOCTL_USB_GET_NODE_INFORMATION, &nodeInfo, nBytes, &nodeInfo, nBytes, &k, NULL))
        {
            hub->HubIsBusPowered = nodeInfo.u.HubInformation.HubIsBusPowered;
            hub->HubPortCount    = nodeInfo.u.HubInformation.HubDescriptor.bNumberOfPorts;
        }

        CloseHandle(h2);
    }

    // Fill in the missing Manufacture, Product, and SerialNumber values
    // values by just creating a Device instance and copying the values
    device = GetPortDevice(port);
    if(device != NULL)
    {
        strncpy(hub->InstanceId, device->InstanceId, USB_BUFFER_SIZE);
        strncpy(hub->Manufacturer, device->Manufacturer, USB_BUFFER_SIZE);
        strncpy(hub->Product, device->Product, USB_BUFFER_SIZE);
        strncpy(hub->SerialNumber, device->SerialNumber, USB_BUFFER_SIZE);
        strncpy(hub->DriverKey, device->DriverKey, USB_BUFFER_SIZE);
        free(device);
    }

    CloseHandle(h);
    return hub;
}

static void SearchHubInstanceId(UsbHub_t* hub, UsbDevice_t** foundDevice, const char* instanceId)
{
    UsbPort_t*   portList;
    DWORD        portCount = 0;
    DWORD        i;
    UsbDevice_t* device;
    UsbHub_t*    childHub;
    *foundDevice = NULL;

    if(hub == NULL) return;

    portList = GetHubPorts(hub, &portCount);

    if(portList == NULL || portCount == 0) return;

    for(i = 0; i < portCount; i++)
    {
        if(portList[i].PortIsHub)
        {
            childHub = GetPortHub(&portList[i]);

            if(childHub == NULL) continue;

            SearchHubInstanceId(childHub, foundDevice, instanceId);

            free(childHub);
        }
        else
        {
            if(!portList[i].PortIsDeviceConnected) continue;

            device = GetPortDevice(&portList[i]);
            if(device == NULL || strncmp(device->InstanceId, instanceId, USB_BUFFER_SIZE) != 0) continue;

            *foundDevice = device;
            break;
        }
    }

    free(portList);
}

UsbDevice_t* FindDeviceByInstanceId(const char* instanceId)
{
    UsbController_t* controllers;
    DWORD            controllers_len = 0;
    DWORD            i;
    UsbDevice_t*     foundDevice = NULL;

    controllers = GetHostControllers(&controllers_len);

    if(controllers == NULL || controllers_len == 0) return NULL;

    for(i = 0; i < controllers_len; i++)
    {
        SearchHubInstanceId(GetRootHub(&controllers[i]), &foundDevice, instanceId);
        if(foundDevice != NULL) break;
    }

    return foundDevice;
}

UsbDevice_t* FindDeviceNumber(DWORD devNum, GUID diskGuid)
{
    HDEVINFO                         h;
    BOOL                             success;
    DWORD                            i = 0;
    SP_DEVICE_INTERFACE_DATA         dia;
    SP_DEVINFO_DATA                  da;
    PSP_DEVICE_INTERFACE_DETAIL_DATA didd;
    DWORD                            nRequiredSize = 0;
    DWORD                            N_BYTES       = USB_BUFFER_SIZE;
    DEVINST                          ptrPrevious;
    PSTR                             instanceId  = malloc(N_BYTES);
    UsbDevice_t*                     foundDevice = NULL; // TODO: Determine type

    didd = malloc(USB_BUFFER_SIZE + sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA));
    if(instanceId) return NULL;

    memset(instanceId, 0, N_BYTES);

    // We start at the "root" of the device tree and look for all
    // devices that match the interface GUID of a disk
    h = SetupDiGetClassDevs(&diskGuid, 0, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

    if(h != INVALID_HANDLE_VALUE)
    {
        do {
            // create a Device Interface Data structure
            memset(&dia, 0, sizeof(SP_DEVICE_INTERFACE_DATA));
            dia.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

            // start the enumeration
            success = SetupDiEnumDeviceInterfaces(h, NULL, &diskGuid, i, &dia);
            if(success)
            {
                // build a DevInfo Data structure
                memset(&da, 0, sizeof(SP_DEVINFO_DATA));
                da.cbSize = sizeof(SP_DEVINFO_DATA);

                // build a Device Interface Detail Data structure
                memset(didd, 0, USB_BUFFER_SIZE + sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA));
                didd->cbSize = 4 + sizeof(TCHAR);

                // now we can get some more detailed information
                nRequiredSize = 0;

                if(SetupDiGetDeviceInterfaceDetail(h, &dia, didd, N_BYTES, &nRequiredSize, &da))
                    if(GetDeviceNumber(didd->DevicePath) == devNum)
                    {
                        // current InstanceID is at the "USBSTOR" level, so we
                        // need up "move up" one level to get to the "USB" level
                        CM_Get_Parent(&ptrPrevious, da.DevInst, 0);

                        // Now we get the InstanceID of the USB level device
                        memset(instanceId, 0, N_BYTES);
                        CM_Get_Device_ID(ptrPrevious, instanceId, N_BYTES, 0);
                    }
            }

            i++;
        } while(success);

        SetupDiDestroyClassImageList(h);
    }

    // Did we find an InterfaceID of a USB device?
    if(strlen(instanceId) > 0 && strncmp(instanceId, "USB\\", 3) == 0) foundDevice = FindDeviceByInstanceId(instanceId);

    free(didd);
    return foundDevice;
}

UsbDevice_t* FindDrivePath(DeviceContext* ctx, GUID deviceGuid)
{
    // We start by getting the unique DeviceNumber of the given
    // DriveLetter.  We'll use this later to find a matching
    // DevicePath "symbolic name"
    DWORD devNum = GetDeviceNumber(ctx->device_path);
    return devNum < 0 ? NULL : FindDeviceNumber(devNum, deviceGuid);
}

uint8_t GetUsbData(void*     device_ctx,
                   uint16_t* desc_len,
                   char*     descriptors,
                   uint16_t* id_vendor,
                   uint16_t* id_product,
                   char*     manufacturer,
                   char*     product,
                   char*     serial)
{
    DeviceContext* ctx    = device_ctx;
    UsbDevice_t*   device = NULL;
    GUID*          guids;
    int            i;

    guids    = malloc(sizeof(GUID) * 4);
    guids[0] = GUID_DEVINTERFACE_FLOPPY;
    guids[1] = GUID_DEVINTERFACE_CDROM;
    guids[2] = GUID_DEVINTERFACE_DISK;
    guids[3] = GUID_DEVINTERFACE_TAPE;

    if(!ctx) return -1;

    for(i = 0; i < 4; i++)
    {
        device = FindDrivePath(ctx, guids[i]);

        if(device != NULL) break;
    }

    if(device == NULL)
    {
        free(guids);
        return 0;
    }

    memcpy(descriptors, device->BinaryDeviceDescriptors, device->BinaryDeviceDescriptorsLength);
    *desc_len   = (uint16_t)device->BinaryDeviceDescriptorsLength;
    *id_vendor  = device->DeviceDescriptor.idVendor;
    *id_product = device->DeviceDescriptor.idProduct;
    strncpy(manufacturer, device->Manufacturer, 256);
    strncpy(product, device->Product, 256);
    strncpy(serial, device->SerialNumber, 256);

    free(guids);

    return 1;
}