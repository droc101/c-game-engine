//
// Created by droc101 on 2/15/26.
//

#ifndef GAME_DPRINTCONSOLE_H
#define GAME_DPRINTCONSOLE_H

/**
 * Add a message to the DPrint console
 * @param msg The message
 * @param color ANSI color code for the message (30-39, 90-97)
 */
void AddConsoleMessage(const char *msg, int color);

/**
 * Initialize the DPrint console
 */
void InitDPrintConsole();

/**
 * Destroy the DPrint console
 */
void DestroyDPrintConsole();

/**
 * Process and draw the DPrint console
 */
void ProcessDPrintConsole();

#endif //GAME_DPRINTCONSOLE_H
