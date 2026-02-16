//
// Created by droc101 on 2/15/26.
//

#ifndef GAME_DPRINTCONSOLE_H
#define GAME_DPRINTCONSOLE_H

#include <engine/structs/Color.h>
#include <stddef.h>

void AddConsoleMessage(const char *msg, Color color);

void InitDPrintConsole();

void DestroyDPrintConsole();

void ProcessDPrintConsole();

#endif //GAME_DPRINTCONSOLE_H
