//
// Created by droc101 on 11/10/2024.
//

#include "PlatformHelpers.h"

#include "../defines.h"

#ifdef WIN32
#include <dwmapi.h>
#include <SDL_syswm.h>
#include "Core/Logging.h"

#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

void DwmDarkMode(SDL_Window *window)
{
#ifdef WIN32
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	SDL_GetWindowWMInfo(window, &info);
	const HWND hWnd = info.info.win.window; // NOLINT(*-misplaced-const)
	const BOOL enable = true;
	const HRESULT res = DwmSetWindowAttribute(hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &enable, sizeof(BOOL));
	if (res != S_OK)
	{
		LogWarning("Failed to enable dark mode: %lx\n", res);
	}
#else
	(void)window;
#endif
}
