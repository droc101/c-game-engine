//
// Created by droc101 on 11/14/2025.
//

#include "../include/DialogSystem.h"
#include <errno.h>
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
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <wchar.h>

extern char **environ;
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
	// ReSharper disable CppPrintfBadFormat
	char *text_argument = malloc(strlen("--text=") + wcslen(message) + 1);
	if (!text_argument)
	{
		printf(ANSI_RED "bootstrap: malloc failed: %s\n" ANSI_RESET, strerror(errno));
		return;
	}
	sprintf(text_argument, "--text=%ls", message);
	char *title_argument = malloc(strlen("--title=") + wcslen(title) + 1);
	if (!title_argument)
	{
		free(text_argument);
		printf(ANSI_RED "bootstrap: malloc failed: %s\n" ANSI_RESET, strerror(errno));
		return;
	}
	sprintf(title_argument, "--title=%ls", title);
	// ReSharper restore CppPrintfBadFormat
	char *zenity_argv[] = {"zenity", "--error", "--no-wrap", "--ok-label=Quit", text_argument, title_argument, NULL};

	const pid_t pid = fork();
	if (pid == -1)
	{
		printf(ANSI_RED "bootstrap: fork() failed: %s" ANSI_RESET, strerror(errno));
		free(text_argument);
		free(title_argument);
		return;
	}
	if (pid == 0) // 0 = child
	{
		if (execvp(zenity_argv[0], zenity_argv) == -1)
		{
			printf(ANSI_RED "bootstrap: execvp() failed: %s\n" ANSI_RESET, strerror(errno));
		}
	} else
	{
		printf("bootstrap: waiting on zenity exit\n");
		waitpid(pid, NULL, 0);
	}

	free(text_argument);
	free(title_argument);

#endif
}
