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

#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#include "../aaruremote.h"

static struct termios original_termios;
static uint8_t        terminal_configured = 0;

static void RestoreTerminalMode()
{
    if(terminal_configured) tcsetattr(STDIN_FILENO, TCSANOW, &original_termios);
}

void Initialize()
{
    struct termios current_termios;

    if(!isatty(STDIN_FILENO)) return;
    if(tcgetattr(STDIN_FILENO, &original_termios)) return;

    current_termios = original_termios;
    current_termios.c_lflag &= ~(ICANON | ECHO);
    current_termios.c_cc[VMIN]  = 0;
    current_termios.c_cc[VTIME] = 0;

    if(!tcsetattr(STDIN_FILENO, TCSANOW, &current_termios))
    {
        terminal_configured = 1;
        atexit(RestoreTerminalMode);
    }
}

void PlatformLoop(AaruPacketHello *pkt_server_hello) { WorkingLoop(pkt_server_hello); }

void PlatformResetIdleInput()
{
    if(terminal_configured) tcflush(STDIN_FILENO, TCIFLUSH);
}

uint8_t PlatformGetIdleAction()
{
    char key;

    if(!terminal_configured) return AARUREMOTE_IDLE_ACTION_NONE;
    if(read(STDIN_FILENO, &key, 1) <= 0) return AARUREMOTE_IDLE_ACTION_NONE;
    if(key == 'd' || key == 'D') return AARUREMOTE_IDLE_ACTION_LIST_DEVICES;

    return AARUREMOTE_IDLE_ACTION_NONE;
}

uint8_t AmIRoot() { return geteuid() == 0; }