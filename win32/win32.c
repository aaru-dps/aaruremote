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

#include "win32.h"

#include "../aaruremote.h"

void Initialize()
{
    // Do nothing
}

void PlatformLoop(AaruPacketHello* pkt_server_hello) { WorkingLoop(pkt_server_hello); }

uint8_t AmIRoot()
{
    BOOL                     b;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID                     AdministratorsGroup;

    b = AllocateAndInitializeSid(
        &NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &AdministratorsGroup);

    if(b)
    {
        if(!CheckTokenMembership(NULL, AdministratorsGroup, &b)) { b = FALSE; }
        FreeSid(AdministratorsGroup);
    }

    return b;
}