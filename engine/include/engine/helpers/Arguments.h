//
// Created by droc101 on 6/22/25.
//

#ifndef ARGUMENTS_H
#define ARGUMENTS_H

#include <stdbool.h>

/**
 * Initialize the argument system with argc and argv from the main function.
 */
void InitArguments(int processArgc, const char **processArgv);

/**
 * Get a CLI argument's value as a string
 * @param argument The argument to get the value of (such as "--game")
 * @param defaultValue The default value to use if the argument is not present
 * @return The value of the argument, or the default if needed
 */
char *GetCliArgStr(const char *argument, char *defaultValue);

/**
* Get a CLI argument's value as an integer
 * @param argument The argument to get the value of (such as "--game")
 * @param defaultValue The default value to use if the argument is not present
 * @return The value of the argument, or the default if needed
 */
int GetCliArgInt(const char *argument, int defaultValue);

/**
* Get a CLI argument's value as a boolean
 * @param argument The argument to get the value of (such as "--game")
 * @param defaultValue The default value to use if the argument is not present
 * @return The value of the argument, or the default if needed
 */
bool GetCliArgBool(const char *argument, bool defaultValue);

/**
 * Check if an argument is present
 * @param argument The argument to check for (such as "--game")
 * @return Whether the argument is present
 */
bool HasCliArg(const char *argument);

#endif //ARGUMENTS_H
