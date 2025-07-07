//
// Created by droc101 on 6/22/25.
//

#ifndef ARGUMENTS_H
#define ARGUMENTS_H
#include <stdbool.h>

char *GetCliArgStr(int argc, char *argv[], const char *argument, char *default_value);

int GetCliArgInt(int argc, char *argv[], const char *argument, int default_value);

bool GetCliArgBool(int argc, char *argv[], const char *argument, bool default_value);

bool HasCliArg(int argc, char *argv[], const char *argument);

#endif //ARGUMENTS_H
