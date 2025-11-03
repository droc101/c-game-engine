//
// Created by droc101 on 6/22/25.
//

#ifndef ARGUMENTS_H
#define ARGUMENTS_H

#include <stdbool.h>

/**
 * Get a CLI argument's value as a string
 * @param argc Argument count
 * @param argv Argument values
 * @param argument The argument to get the value of (such as "--game")
 * @param default_value The default value to use if the argument is not present
 * @return The value of the argument, or the default if needed
 */
char *GetCliArgStr(int argc, const char *argv[], const char *argument, char *default_value);

/**
* Get a CLI argument's value as an integer
 * @param argc Argument count
 * @param argv Argument values
 * @param argument The argument to get the value of (such as "--game")
 * @param default_value The default value to use if the argument is not present
 * @return The value of the argument, or the default if needed
 */
int GetCliArgInt(int argc, const char *argv[], const char *argument, int default_value);

/**
* Get a CLI argument's value as a boolean
 * @param argc Argument count
 * @param argv Argument values
 * @param argument The argument to get the value of (such as "--game")
 * @param default_value The default value to use if the argument is not present
 * @return The value of the argument, or the default if needed
 */
bool GetCliArgBool(int argc, const char *argv[], const char *argument, bool default_value);

/**
 * Check if an argument is present
 * @param argc Argument count
 * @param argv Argument value
 * @param argument The argument to check for (such as "--game")
 * @return Whether the argument is present
 */
bool HasCliArg(int argc, const char *argv[], const char *argument);

#endif //ARGUMENTS_H
