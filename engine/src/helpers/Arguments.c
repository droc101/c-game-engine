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

char *GetCliArgStr(const int argc, const char *argv[], const char *argument, char *default_value)
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
	return default_value;
}

int GetCliArgInt(const int argc, const char *argv[], const char *argument, const int default_value)
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
	return default_value;
}

bool GetCliArgBool(const int argc, const char *argv[], const char *argument, const bool default_value)
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
	return default_value;
}

bool HasCliArg(const int argc, const char *argv[], const char *argument)
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
