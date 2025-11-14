//
// Created by droc101 on 11/14/2025.
//

#include "../include/DialogSystem.h"

#ifdef WIN32
#include <stdio.h>
#include "../include/AnsiCodes.h"
// clang-format off
#include <windows.h> // This include must be above commctrl.h otherwise there are compile errors.
#include <commctrl.h>
// clang-format on
#else
#include <wchar.h>
#endif

void InitDialogSystem()
{
#ifdef WIN32
	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	InitCommonControlsEx(&icex);
#endif
}

void ErrorDialog(const wchar_t *instruction, const wchar_t *message, const wchar_t *title)
{
#ifdef WIN32
	int buttonPressed = 0;
	const HRESULT
			res = TaskDialog(NULL, NULL, title, instruction, message, TDCBF_OK_BUTTON, TD_ERROR_ICON, &buttonPressed);
	if (!SUCCEEDED(res))
	{
		printf(ANSI_RED "bootstrap: failed to show error message box HRESULT=%lx\n" ANSI_RESET, res);
	}
#else
	// TODO: linux
	(void)instruction;
	(void)message;
	(void)title;
#endif
}
