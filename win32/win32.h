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

#ifndef AARUREMOTE_WIN32_WIN32_H_
#define AARUREMOTE_WIN32_WIN32_H_
#ifdef _WIN32

typedef __int8           int8_t;
typedef __int16          int16_t;
typedef __int32          int32_t;
typedef __int64          int64_t;
typedef unsigned __int8  uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;

typedef int socklen_t;

#define PROCESSOR_ARCHITECTURE_INTEL 0
#define PROCESSOR_ARCHITECTURE_MIPS 1
#define PROCESSOR_ARCHITECTURE_ALPHA 2
#define PROCESSOR_ARCHITECTURE_PPC 3
#define PROCESSOR_ARCHITECTURE_SHX 4
#define PROCESSOR_ARCHITECTURE_ARM 5
#define PROCESSOR_ARCHITECTURE_IA64 6
#define PROCESSOR_ARCHITECTURE_ALPHA64 7
#define PROCESSOR_ARCHITECTURE_MSIL 8
#define PROCESSOR_ARCHITECTURE_AMD64 9
#define PROCESSOR_ARCHITECTURE_IA32_ON_WIN64 10
#define PROCESSOR_ARCHITECTURE_ARM64 12
#define PROCESSOR_ARCHITECTURE_ARM32_ON_WIN64 13
#define PROCESSOR_ARCHITECTURE_IA32_ON_ARM64 14

typedef struct
{
    SOCKET socket;
} NetworkContext;

typedef struct
{
    HANDLE handle;
    char   device_path[4096];
} DeviceContext;

#ifndef SM_SERVERR2
#define SM_SERVERR2 89
#endif

#ifndef VER_SUITE_WH_SERVER
#define VER_SUITE_WH_SERVER 0x00008000
#endif

#endif // _WIN32
#endif // AARUREMOTE_WIN32_WIN32_H_
