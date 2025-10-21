//
// Created by droc101 on 11/10/2024.
//

#include <engine/helpers/PlatformHelpers.h>
#include <SDL_video.h>
#include <stdlib.h>
#include <unistd.h>
#include <engine/structs/GlobalState.h>
#include <engine/subsystem/Logging.h>

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
	char *args[] = {GetState()->executablePath, NULL};
	execv(GetState()->executablePath, args);
#endif
	exit(1);
}
