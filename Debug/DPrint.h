//
// Created by droc101 on 4/26/2024.
//

#ifndef GAME_DPRINT_H
#define GAME_DPRINT_H

#include <stdbool.h>
#include "../Structs/Color.h"

// Show debug print statements in the top left corner
#define ENABLE_DEBUG_PRINT

/**
 * Resets the DPrint Y position to the top of the screen
 * @note Should only be called once at the start of the physicsFrame
 */
void ResetDPrintYPos();

/**
 * Prints a string to the screen and optionally to the console (stdout)
 * @param str Format string
 * @param color Text color
 * @param con Whether to print to the console
 * @param ... Parameters to be formatted
 */
void DPrintF(const char *str, Color color, bool con, ...);

#endif //GAME_DPRINT_H
