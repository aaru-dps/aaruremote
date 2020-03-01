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

#include "../aaruremote.h"

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

#endif // AARUREMOTE_WIN32_WIN32_H_
