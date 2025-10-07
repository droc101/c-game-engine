//
// Created by droc101 on 11/10/2024.
//

#include "PlatformHelpers.h"
#include <SDL_video.h>

#ifdef WIN32
#include <dwmapi.h>
#include <SDL_syswm.h>
#include "Core/Logging.h"

void SetDwmWindowAttribs(SDL_Window *window)
{
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
}
#else
void DwmDarkMode(SDL_Window *window)
{
	(void)window;
}
#endif
