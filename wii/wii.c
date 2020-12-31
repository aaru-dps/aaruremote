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

#include <debug.h>
#include <errno.h>
#include <gccore.h>
#include <wiiuse/wpad.h>

#include "../aaruremote.h"

void Initialize()
{
    void*              framebuffer;
    static GXRModeObj* rmode = NULL;

    VIDEO_Init();
    WPAD_Init();

    rmode       = VIDEO_GetPreferredMode(NULL);
    framebuffer = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
    console_init(framebuffer, 20, 20, rmode->fbWidth, rmode->xfbHeight, rmode->fbWidth * VI_DISPLAY_PIX_SZ);

    VIDEO_Configure(rmode);
    VIDEO_SetNextFramebuffer(framebuffer);
    VIDEO_SetBlack(FALSE);
    VIDEO_Flush();
    VIDEO_WaitVSync();
    if(rmode->viTVMode & VI_NON_INTERLACE) VIDEO_WaitVSync();
}

void PlatformLoop(AaruPacketHello* pkt_server_hello)
{
    static lwp_t worker = (lwp_t)NULL;
    int          buttonsDown;
    LWP_CreateThread(&worker,          /* thread handle */
                     WorkingLoop,      /* code */
                     pkt_server_hello, /* arg pointer for thread */
                     NULL,             /* stack base */
                     16 * 1024,        /* stack size */
                     50 /* thread priority */);

    while(true)
    {
        VIDEO_WaitVSync();
        WPAD_ScanPads();

        buttonsDown = WPAD_ButtonsDown(0);

        if(buttonsDown & WPAD_BUTTON_HOME) { return; }
    }
}

uint8_t AmIRoot() { return 1; }