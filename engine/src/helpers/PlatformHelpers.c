//
// Created by droc101 on 11/10/2024.
//

#include <ctype.h>
#include <engine/helpers/PlatformHelpers.h>
#include <engine/structs/GlobalState.h>
#include <engine/subsystem/Logging.h>
#include <SDL_video.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef WIN32
#include <dwmapi.h>
#include <handleapi.h>
#include <minwindef.h>
#include <processthreadsapi.h>
#include <SDL_syswm.h>
#include <winbase.h>
#endif

void SetDwmWindowAttribs(SDL_Window *window)
{
#ifdef WIN32
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	SDL_GetWindowWMInfo(window, &info);
	const HWND hWnd = info.info.win.window; // NOLINT(*-misplaced-const)
	const BOOL enable = true;
	HRESULT res = DwmSetWindowAttribute(hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &enable, sizeof(BOOL));
	if (res != S_OK)
	{
		LogWarning("Failed to enable dark mode: %lx\n", res);
	}
	const DWORD cornerPreference = DWMWCP_DONOTROUND;
	res = DwmSetWindowAttribute(hWnd, DWMWA_WINDOW_CORNER_PREFERENCE, &cornerPreference, sizeof(DWORD));
	if (res != S_OK)
	{
		LogWarning("Failed to set window corner preference: %lx\n", res);
	}
#else
	(void)window;
#endif
}

_Noreturn void RestartProgram()
{
	LogWarning("Exiting early to restart engine, resources may not get cleaned properly.\n"); // TODO clean properly
#ifdef WIN32
	STARTUPINFO si = {0};
	PROCESS_INFORMATION pi = {0};
	si.cb = sizeof(si);
	CreateProcess(
			GetState()->executablePath,
			NULL,
			NULL,
			NULL,
			FALSE,
			CREATE_NEW_CONSOLE, // If this is not present it will almost certainly freeze during init. Thank you windows.
			NULL,
			NULL,
			&si,
			&pi);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
#else
	char *args[] = {GetState()->executablePath, NULL}; // TODO use argv and argc now stored in Argument.c
	execv(GetState()->executablePath, args);
#endif
	exit(1);
}

bool IsPathAbsolute(const char *path)
{
	if (path == NULL)
	{
		return false;
	}
#ifdef WIN32
	// yes, the drive "letter" doesn't have to be a letter.
	// no, I'm not accounting for that case.
	// I have also chosen to not care about UNC paths.
	if (strlen(path) < 3 && isalpha(path[0]) && path[1] == ':' && (path[2] == '/' || path[2] == '\\'))
	{
		return true;
	}
#else
	if (path[0] == '/')
	{
		return true;
	}
#endif

	return false;
}
