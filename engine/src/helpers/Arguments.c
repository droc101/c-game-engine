//
// Created by droc101 on 6/22/25.
//

#include <assert.h>
#include <ctype.h>
#include <engine/helpers/Arguments.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

static int argc;
static const char **argv;

void InitArguments(const int processArgc, const char **processArgv)
{
	argc = processArgc;
	argv = processArgv;
}

char *GetCliArgStr(const char *argument, char *defaultValue)
{
	for (int i = 0; i < argc; i++)
	{
		if (strncmp(argv[i], argument, strlen(argument)) == 0)
		{
			char *value = strchr(argv[i], '=');
			if (value != NULL)
			{
				return value + 1;
			}
		}
	}
	return defaultValue;
}

int GetCliArgInt(const char *argument, const int defaultValue)
{
	for (int i = 0; i < argc; i++)
	{
		if (strncmp(argv[i], argument, strlen(argument)) == 0)
		{
			const char *value = strchr(argv[i], '=');
			if (value != NULL)
			{
				return (int)strtol(value + 1, NULL, 10);
			}
		}
	}
	return defaultValue;
}

bool GetCliArgBool(const char *argument, const bool defaultValue)
{
	for (int i = 0; i < argc; i++)
	{
		if (strncmp(argv[i], argument, strlen(argument)) == 0)
		{
			char *value = strchr(argv[i], '=');
			assert(value);
			const size_t length = strlen(value);
			for (size_t j = 0; j < length; j++)
			{
				value[j] = (char)tolower(value[j]);
			}
			if (strncmp(value, "true", strlen("true")) == 0 ||
				strncmp(value, "on", strlen("on")) == 0 ||
				strncmp(value, "yes", strlen("yes")) == 0)
			{
				return true;
			}
			if (strncmp(value, "false", strlen("false")) == 0 ||
				strncmp(value, "off", strlen("off")) == 0 ||
				strncmp(value, "no", strlen("no")) == 0)
			{
				return false;
			}
		}
	}
	return defaultValue;
}

bool HasCliArg(const char *argument)
{
	for (int i = 0; i < argc; i++)
	{
		if (strncmp(argv[i], argument, strlen(argument)) == 0)
		{
			return true;
		}
	}
	return false;
}
