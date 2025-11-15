//
// Created by droc101 on 11/8/25.
//
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include "../include/AnsiCodes.h"
#include "../include/DialogSystem.h"
#include "../include/Filesystem.h"
#include "../include/LibraryLoader.h"

#ifdef __x86_64__
#include "../include/x86.h"
#endif

/// The signature to the entry point in the game shared libraries
typedef int (*GameMainFunction)(int argc, const char *argv[]);

int main(const int argc, const char *argv[])
{
	InitDialogSystem();

	char *argvZero = strdup(argv[0]); // This gets modified on Linux
	const char *directory = GetDirectoryOfFile(argvZero);
	if (directory)
	{
		printf("bootstrap: executable directory is \"%s\"\n", directory);
		ChangeDirectory(directory);
	} else
	{
		printf(ANSI_RED "bootstrap: failed to get executable directory!\n" ANSI_RESET);
		return 1;
	}
	free(argvZero);

	const char *library_basename = NULL;
#ifdef __x86_64__
	uint8_t abiLevel = GetX86AbiArgument(argc, argv);

	if (abiLevel == 0)
	{
		abiLevel = GetX86AbiLevel();
	} else
	{
		printf("bootstrap: forcing abi level %d\n", abiLevel);
	}

	library_basename = GetX86LibraryName(abiLevel);
#else
	library_basename = LIB_PREFIX "game.arm64" LIB_SUFFIX;
#endif

	if (!library_basename)
	{
		printf(ANSI_RED "bootstrap: failed to find a compatible game library\n" ANSI_RESET);
		ErrorDialog(L"Fatal Error",
					L"Failed to find a game executable compatible with your system's CPU.",
					L"Fatal Error");
		return -1;
	}

	printf("bootstrap: using game library \"%s\"\n", library_basename);
	LibraryLoaderSetup();
	const LibraryHandle library = OpenLibrary(library_basename);
	if (library)
	{
		const GameMainFunction GameMain = OpenSymbol(library, "GameMain");
		if (GameMain)
		{
			const int ret = GameMain(argc, argv);
			printf("bootstrap: cleaning up\n");
			CloseLibrary(library);
			return ret;
		}
	}
	printf(ANSI_RED "bootstrap: error loading game library: %s\n" ANSI_RESET, LibraryLoaderError());
	ErrorDialog(L"Fatal Error", L"Failed to load the game executable.", L"Fatal Error");

	return -1;
}
