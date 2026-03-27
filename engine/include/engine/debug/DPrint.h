//
// Created by droc101 on 4/26/2024.
//

#ifndef GAME_DPRINT_H
#define GAME_DPRINT_H

#include <engine/structs/Color.h>
#include <stdbool.h>

// Show debug print statements in the top left corner
#define ENABLE_DEBUG_PRINT

/**
 * Resets the DPrint Y position to the top of the screen
 * @note Should only be called once at the start of the physicsFrame
 */
void ResetDPrintYPos();

void DPrint(const char *str, Color color);

/**
 * Prints a string to the screen and optionally to the console (stdout)
 * @param format Format string
 * @param con Whether to print to the console
 * @param color Text color
 * @param ... Parameters to be formatted
 */
void DPrintF(const char *format, bool printToConsole, Color color, ...);

#endif //GAME_DPRINT_H
