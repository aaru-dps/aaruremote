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

#include <malloc.h>
#include <stdio.h>
#include <string.h>

#include "win32.h"
#include "../aaruremote.h"
#include "../endian.h"

AaruPacketHello* GetHello()
{
    AaruPacketHello*  pkt_server_hello;
    OSVERSIONINFOEX   osvi;
    SYSTEM_INFO       sysinfo;
    UINT              verSize;
    DWORD             unused;
    LPVOID            verData;
    VS_FIXEDFILEINFO* fileVerInfo;

    const char* win     = "Windows";
    const char* w95     = "Windows 95";
    const char* w95osr  = "Windows 95 OSR2";
    const char* w98     = "Windows 98";
    const char* w98se   = "Windows 98 SE";
    const char* wme     = "Windows Me";
    const char* wnt     = "Windows NT";
    const char* w2k     = "Windows 2000";
    const char* wxp     = "Windows XP";
    const char* w2k3    = "Windows Server 2003";
    const char* whs     = "Windows Home Server";
    const char* w2k3r2  = "Windows Server 2003 R2";
    const char* wvista  = "Windows Vista";
    const char* w2k8    = "Windows Server 2008";
    const char* w2k8r2  = "Windows Server 2008 R2";
    const char* w7      = "Windows 7";
    const char* w2k12   = "Windows Server 2012";
    const char* w8      = "Windows 8";
    const char* w2k12r2 = "Windows Server 2012 R2";
    const char* w81     = "Windows 8.1";
    const char* w2k16   = "Windows Server 2016";
    const char* w10     = "Windows 10";

    pkt_server_hello = malloc(sizeof(AaruPacketHello));

    if(!pkt_server_hello) return 0;

    memset(pkt_server_hello, 0, sizeof(AaruPacketHello));

    pkt_server_hello->hdr.remote_id   = htole32(AARUREMOTE_REMOTE_ID);
    pkt_server_hello->hdr.packet_id   = htole32(AARUREMOTE_PACKET_ID);
    pkt_server_hello->hdr.len         = htole32(sizeof(AaruPacketHello));
    pkt_server_hello->hdr.version     = AARUREMOTE_PACKET_VERSION;
    pkt_server_hello->hdr.packet_type = AARUREMOTE_PACKET_TYPE_HELLO;
    strncpy(pkt_server_hello->application, AARUREMOTE_NAME, sizeof(AARUREMOTE_NAME));
    strncpy(pkt_server_hello->version, AARUREMOTE_VERSION, sizeof(AARUREMOTE_VERSION));
    pkt_server_hello->max_protocol = AARUREMOTE_PROTOCOL_MAX;

    ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    GetVersionEx((LPOSVERSIONINFOA)&osvi);

    if(osvi.dwPlatformId == VER_PLATFORM_WIN32s)
    {
        strncpy(pkt_server_hello->sysname, win, 255);
        sprintf_s(pkt_server_hello->release, 255, "%d.%02d", osvi.dwMajorVersion, osvi.dwMinorVersion);
        strncpy(pkt_server_hello->machine, "x86", 255);

        return pkt_server_hello;
    }

    if(osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
    {
        switch(osvi.dwMinorVersion)
        {
            case 0:
                if(strlen(osvi.szCSDVersion) > 0) strncpy(pkt_server_hello->sysname, w95osr, 255);
                else
                    strncpy(pkt_server_hello->sysname, w95, 255);
                break;
            case 10:
                if(strlen(osvi.szCSDVersion) > 0) strncpy(pkt_server_hello->sysname, w98se, 255);
                else
                    strncpy(pkt_server_hello->sysname, w98, 255);
                break;
            case 90: strncpy(pkt_server_hello->sysname, wme, 255); break;
            default: strncpy(pkt_server_hello->sysname, win, 255); break;
        }

        sprintf_s(pkt_server_hello->release,
                  255,
                  "%d.%02d.%d",
                  osvi.dwMajorVersion,
                  osvi.dwMinorVersion,
                  osvi.dwBuildNumber & 0xFFFF);
        strncpy(pkt_server_hello->machine, "x86", 255);

        return pkt_server_hello;
    }

    ZeroMemory(&sysinfo, sizeof(SYSTEM_INFO));
    GetSystemInfo(&sysinfo);

    switch(sysinfo.wProcessorArchitecture)
    {
        case PROCESSOR_ARCHITECTURE_IA32_ON_WIN64:
        case PROCESSOR_ARCHITECTURE_IA32_ON_ARM64:
        case PROCESSOR_ARCHITECTURE_INTEL: strncpy(pkt_server_hello->machine, "x86", 255); break;
        case PROCESSOR_ARCHITECTURE_MIPS: strncpy(pkt_server_hello->machine, "mips", 255); break;
        case PROCESSOR_ARCHITECTURE_ALPHA:
        case PROCESSOR_ARCHITECTURE_ALPHA64: strncpy(pkt_server_hello->machine, "axp", 255); break;
        case PROCESSOR_ARCHITECTURE_PPC: strncpy(pkt_server_hello->machine, "ppc", 255); break;
        case PROCESSOR_ARCHITECTURE_SHX: strncpy(pkt_server_hello->machine, "sh", 255); break;
        case PROCESSOR_ARCHITECTURE_ARM32_ON_WIN64:
        case PROCESSOR_ARCHITECTURE_ARM: strncpy(pkt_server_hello->machine, "arm", 255); break;
        case PROCESSOR_ARCHITECTURE_IA64: strncpy(pkt_server_hello->machine, "ia64", 255); break;
        case PROCESSOR_ARCHITECTURE_MSIL: strncpy(pkt_server_hello->machine, "msil", 255); break;
        case PROCESSOR_ARCHITECTURE_AMD64: strncpy(pkt_server_hello->machine, "x64", 255); break;
        case PROCESSOR_ARCHITECTURE_ARM64: strncpy(pkt_server_hello->machine, "aarch64", 255); break;
        default: strncpy(pkt_server_hello->machine, "unknown", 255); break;
    }

    if(strlen(osvi.szCSDVersion) > 0)
        sprintf_s(pkt_server_hello->release,
                  255,
                  "%d.%d.%d (%s)",
                  osvi.dwMajorVersion,
                  osvi.dwMinorVersion * 10,
                  osvi.dwBuildNumber,
                  osvi.szCSDVersion);
    else
        sprintf_s(pkt_server_hello->release,
                  255,
                  "%d.%d.%d",
                  osvi.dwMajorVersion,
                  osvi.dwMinorVersion * 10,
                  osvi.dwBuildNumber);

    if(osvi.dwMajorVersion < 5)
    {
        strncpy(pkt_server_hello->sysname, wnt, 255);
        return pkt_server_hello;
    }

    if(osvi.dwMajorVersion == 5)
    {
        if(osvi.dwMinorVersion == 0)
        {
            strncpy(pkt_server_hello->sysname, w2k, 255);
            return pkt_server_hello;
        }

        if(osvi.dwMinorVersion == 1)
        {
            strncpy(pkt_server_hello->sysname, wxp, 255);
            return pkt_server_hello;
        }

        if((osvi.wProductType == VER_NT_WORKSTATION) &&
           (sysinfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64))
        {
            strncpy(pkt_server_hello->sysname, wxp, 255);
            return pkt_server_hello;
        }

        if(osvi.wSuiteMask & VER_SUITE_WH_SERVER)
        {
            strncpy(pkt_server_hello->sysname, whs, 255);
            return pkt_server_hello;
        }

        if(GetSystemMetrics(SM_SERVERR2) == 0)
        {
            strncpy(pkt_server_hello->sysname, w2k3, 255);
            return pkt_server_hello;
        }
        else
        {
            strncpy(pkt_server_hello->sysname, w2k3r2, 255);
            return pkt_server_hello;
        }
    }

    if(osvi.dwMajorVersion == 6)
    {
        if(osvi.dwMinorVersion == 0)
        {
            if(osvi.wProductType == VER_NT_WORKSTATION)
            {
                strncpy(pkt_server_hello->sysname, wvista, 255);
                return pkt_server_hello;
            }
            strncpy(pkt_server_hello->sysname, w2k8, 255);
            return pkt_server_hello;
        }

        if(osvi.dwMinorVersion == 1)
        {
            if(osvi.wProductType == VER_NT_WORKSTATION)
            {
                strncpy(pkt_server_hello->sysname, w7, 255);
                return pkt_server_hello;
            }
            strncpy(pkt_server_hello->sysname, w2k8r2, 255);
            return pkt_server_hello;
        }

        if(osvi.dwMinorVersion == 2)
        {
            verSize = GetFileVersionInfoSizeA("kernel32.dll", &unused);

            if(verSize > 0)
            {
                verData = malloc(verSize);

                if(verData != NULL)
                {
                    memset(verData, 0, verSize);

                    if(GetFileVersionInfoA("kernel32.dll", 0, verSize, verData) &&
                       VerQueryValueA(verData, "\\", (LPVOID*)&fileVerInfo, &verSize))
                    {
                        osvi.dwMajorVersion = HIWORD(fileVerInfo->dwProductVersionMS);
                        osvi.dwMinorVersion = LOWORD(fileVerInfo->dwProductVersionMS);
                        osvi.dwBuildNumber  = HIWORD(fileVerInfo->dwProductVersionLS);

                        if(strlen(osvi.szCSDVersion) > 0)
                            sprintf_s(pkt_server_hello->release,
                                      255,
                                      "%d.%d.%d (%s)",
                                      osvi.dwMajorVersion,
                                      osvi.dwMinorVersion * 10,
                                      osvi.dwBuildNumber,
                                      osvi.szCSDVersion);
                        else
                            sprintf_s(pkt_server_hello->release,
                                      255,
                                      "%d.%d.%d",
                                      osvi.dwMajorVersion,
                                      osvi.dwMinorVersion * 10,
                                      osvi.dwBuildNumber);
                    }

                    free(verData);
                }
            }

            if(osvi.dwMajorVersion == 6)
            {
                if(osvi.dwMinorVersion == 2)
                {
                    if(osvi.wProductType == VER_NT_WORKSTATION)
                    {
                        strncpy(pkt_server_hello->sysname, w2k12, 255);
                        return pkt_server_hello;
                    }
                    strncpy(pkt_server_hello->sysname, w8, 255);
                    return pkt_server_hello;
                }

                if(osvi.dwMinorVersion == 3)
                {
                    if(osvi.wProductType == VER_NT_WORKSTATION)
                    {
                        strncpy(pkt_server_hello->sysname, w2k12r2, 255);
                        return pkt_server_hello;
                    }
                    strncpy(pkt_server_hello->sysname, w81, 255);
                    return pkt_server_hello;
                }
            }

            if(osvi.dwMajorVersion == 10)
            {
                if(osvi.wProductType == VER_NT_WORKSTATION)
                {
                    strncpy(pkt_server_hello->sysname, w2k16, 255);
                    return pkt_server_hello;
                }
                strncpy(pkt_server_hello->sysname, w10, 255);
                return pkt_server_hello;
            }
        }
    }

    strncpy(pkt_server_hello->sysname, wnt, 255);
    return pkt_server_hello;
}
