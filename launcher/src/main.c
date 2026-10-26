//
// Created by droc101 on 11/8/25.
//
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/AnsiCodes.h"
#include "../include/DialogSystem.h"
#include "../include/Filesystem.h"
#include "../include/LibraryLoader.h"
#include "../include/Lock.h"

#ifdef __x86_64__
#include "../include/x86.h"
#endif

/// The signature to the entry point in the game shared libraries
typedef int (*GameMainFunction)(int argc, char **argv);

int main(const int argc, char **argv)
{
#ifdef WIN32

#ifndef NDEBUG
	// Create a terminal window if not already attached to a terminal
	if (GetStdHandle(STD_OUTPUT_HANDLE) == NULL)
	{
		if (AllocConsole())
		{
			// Reopen stdio handles to point to newly created terminal
			freopen("CONOUT$", "w", stdout);
			freopen("CONOUT$", "w", stderr);
			freopen("CONIN$", "r", stdin);
		}
	}
#endif

	// Enable processing of ANSI escape sequences
	SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), ENABLE_VIRTUAL_TERMINAL_PROCESSING | ENABLE_PROCESSED_OUTPUT);
	SetConsoleMode(GetStdHandle(STD_ERROR_HANDLE), ENABLE_VIRTUAL_TERMINAL_PROCESSING | ENABLE_PROCESSED_OUTPUT);
#endif

	InitDialogSystem();

	if (argc < 1)
	{
		printf(ANSI_RED "bootstrap: argc < 1\n" ANSI_RESET);
		ErrorDialog("Missing executable path argument", "Fatal Error");
		return -1;
	}

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

	if (!CreateMultiInstanceLock())
	{
		printf(ANSI_RED "bootstrap: multi-instance check failed\n" ANSI_RESET);
		ErrorDialog("Only one instance can be open at a time", "Already Running");
		return -1;
	}

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
		DestroyMultiInstanceLock();
		printf(ANSI_RED "bootstrap: failed to find a compatible game library\n" ANSI_RESET);
		ErrorDialog("Failed to find a game executable compatible with your system's CPU.", "Fatal Error");
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
			DestroyMultiInstanceLock();
			return ret;
		}
	}

	DestroyMultiInstanceLock();
	printf(ANSI_RED "bootstrap: error loading game library: %s\n" ANSI_RESET, LibraryLoaderError());
	ErrorDialog("Failed to load the game executable.", "Fatal Error");

	return -1;
}
