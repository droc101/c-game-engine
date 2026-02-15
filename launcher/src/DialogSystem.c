//
// Created by droc101 on 11/14/2025.
//

#include "../include/DialogSystem.h"
#include "../include/AnsiCodes.h"

#ifdef WIN32
#include <stdio.h>
#include "../include/AnsiCodes.h"
// clang-format off
#include <windows.h> // This include must be above commctrl.h otherwise there are compile errors.
#include <commctrl.h>
// clang-format on
#else
#include <sched.h>
#include <spawn.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
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

void ErrorDialog(const wchar_t *message, const wchar_t *title)
{
#ifdef WIN32
	int buttonPressed = 0;
	const HRESULT res = TaskDialog(NULL, NULL, title, title, message, TDCBF_OK_BUTTON, TD_ERROR_ICON, &buttonPressed);
	if (!SUCCEEDED(res))
	{
		printf(ANSI_RED "bootstrap: failed to show error message box HRESULT=%lx\n" ANSI_RESET, res);
	}
#else
	char *text_argument = malloc(strlen("--text=") + wcslen(message) + 1);
	sprintf(text_argument, "--text=%ls", message);
	char *title_argument = malloc(strlen("--title=") + wcslen(title) + 1);
	sprintf(title_argument, "--title=%ls", title);
	char *zenity_argv[] = {"zenity", "--error", "--no-wrap", "--ok-label=Quit", text_argument, title_argument, NULL};

	pid_t zenity_pid = 0;
	if (posix_spawnp(&zenity_pid, "zenity", NULL, NULL, zenity_argv, __environ) == 0)
	{
		waitpid(zenity_pid, NULL, 0);
	} else
	{
		printf(ANSI_RED "bootstrap: failed to show error dialog via zenity!\n" ANSI_RESET);
	}

	free(text_argument);
	free(title_argument);

#endif
}
