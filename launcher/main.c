//
// Created by droc101 on 11/8/25.
//
#include <stdint.h>
#include <stdio.h>
#include "LibraryLoader.h"

#define ANSI_RED "\x1b[31m"
#define ANSI_YELLOW "\x1b[33m"
#define ANSI_RESET "\x1b[39m"

#ifdef WIN32
#include <windows.h> // This include must be above commctrl.h otherwise there are compile errors.
#include <commctrl.h>

#define LIB_SUFFIX ".dll"
#define LIB_PREFIX ""
#else
#define LIB_SUFFIX ".so"
#define LIB_PREFIX "./" // TODO don't assume work dir = binary dir
#endif

typedef int (*GameMainFunction)(int argc, const char *argv[]);

void ErrorMessageBox(const wchar_t *instruction, const wchar_t *message, const wchar_t *title)
{
#ifdef WIN32
	int buttonPressed = 0;
	const HRESULT
			res = TaskDialog(NULL, NULL, title, instruction, message, TDCBF_OK_BUTTON, TD_ERROR_ICON, &buttonPressed);
	if (!SUCCEEDED(res))
	{
		printf(ANSI_RED "bootstrap: failed to show error message box HRESULT=%lx\n" ANSI_RESET, res);
	}
#endif
	// TODO linux
}

int main(const int argc, const char *argv[])
{
#ifdef WIN32
	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	InitCommonControlsEx(&icex);
#endif

	const char *library_basename = NULL;
#ifdef __x86_64__
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
#else
	library_basename = LIB_PREFIX "game.arm64" LIB_SUFFIX;
#endif

	if (!library_basename)
	{
		printf(ANSI_RED "bootstrap: failed to find a compatible game library\n" ANSI_RESET);
		ErrorMessageBox(L"Fatal Error",
						L"Failed to find a game executable compatible with your system's CPU.",
						L"Fatal Error");
		return -1;
	}

	printf("bootstrap: using game library \"%s\"\n", library_basename);
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
	ErrorMessageBox(L"Fatal Error", L"Failed to load the game executable.", L"Fatal Error");
	printf(ANSI_RED "bootstrap: error loading game library: %s\n" ANSI_RESET, LibraryLoaderError());
	return -1;
}
