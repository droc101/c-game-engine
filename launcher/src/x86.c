//
// Created by droc101 on 11/14/25.
//

#include "../include/x86.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/AnsiCodes.h"
#include "../include/LibraryLoader.h"

uint8_t GetX86AbiArgument(const int argc, const char *argv[])
{
	for (int i = 0; i < argc; i++)
	{
		if (strncmp(argv[i], "--abi-level", strlen("--abi-level")) == 0)
		{
			const char *value = strchr(argv[i], '=');
			if (value != NULL)
			{
				const int level = (int)strtol(value + 1, NULL, 10);
				if (level > 0 && level <= 4)
				{
					return level;
				}
				printf(ANSI_YELLOW "bootstrap: --abi-level was set, but the value was not valid.\n" ANSI_RESET);
			}
		}
	}
	return 0;
}

uint8_t GetX86AbiLevel()
{
	uint8_t abiLevel = 0;
	__builtin_cpu_init();
	if (__builtin_cpu_supports("x86-64-v4"))
	{
		abiLevel = 4;
	} else if (__builtin_cpu_supports("x86-64-v3"))
	{
		abiLevel = 3;
	} else if (__builtin_cpu_supports("x86-64-v2"))
	{
		abiLevel = 2;
	} else if (__builtin_cpu_supports("x86-64"))
	{
		abiLevel = 1;
	}
	return abiLevel;
}

const char *GetX86LibraryName(uint8_t abiLevel)
{
	const char *library_basename = NULL;
	while (abiLevel > 0)
	{
		const char *libraries[4] = {
			LIB_PREFIX "game.x86v1" LIB_SUFFIX,
			LIB_PREFIX "game.x86v2" LIB_SUFFIX,
			LIB_PREFIX "game.x86v3" LIB_SUFFIX,
			LIB_PREFIX "game.x86v4" LIB_SUFFIX,
		};
		FILE *lib = fopen(libraries[abiLevel - 1], "r");
		if (lib)
		{
			fclose(lib);
			library_basename = libraries[abiLevel - 1];
			break;
		}
		printf(ANSI_YELLOW "bootstrap: tried to load library \"%s\", but could not open it\n" ANSI_RESET,
			   libraries[abiLevel - 1]);
		abiLevel--;
	}
	return library_basename;
}
