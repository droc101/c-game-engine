//
// Created by droc101 on 4/26/2024.
//

#ifndef GAME_DPRINT_H
#define GAME_DPRINT_H

#include <engine/structs/Color.h>
#include <stdint.h>

/**
 * Resets the DPrint Y position to the top of the screen
 * @note Should only be called once at the start of the physicsFrame
 */
void ResetDPrintYPos();

/**
 * Print a string to the screen
 * @param str The string to print
 * @param color The color to print with
 */
void DPrint(const char *str, Color color);

/**
 * Prints a string to the screen and optionally to the console (stdout)
 * @param format Format string
 * @param color Text color
 * @param ... Parameters to be formatted
 */
void DPrintF(const char *format, Color color, ...);

void DPrintSpacing(uint32_t spacing);

#endif //GAME_DPRINT_H
