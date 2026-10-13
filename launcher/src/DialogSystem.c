//
// Created by droc101 on 11/14/2025.
//

#include <errno.h>

#include "../include/AnsiCodes.h"
#include "../include/DialogSystem.h"

#ifdef WIN32
#include <stdio.h>
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

static const wchar_t *ToWideChar(const char *string)
{
	const size_t stringSize = strlen(string) + 1;
	wchar_t *wideString = malloc(sizeof(wchar_t) * stringSize);
	mbstowcs(wideString, string, stringSize);
	return wideString;
}

void ErrorDialog(const char *message, const char *title)
{
#ifdef WIN32
	wchar_t *wideTitle = ToWideChar(title);
	wchar_t *wideMessage = ToWideChar(message);

	int buttonPressed = 0;
	const HRESULT res = TaskDialog(NULL,
								   NULL,
								   wideTitle,
								   wideTitle,
								   wideMessage,
								   TDCBF_OK_BUTTON,
								   TD_ERROR_ICON,
								   &buttonPressed);
	if (!SUCCEEDED(res))
	{
		printf(ANSI_RED "bootstrap: failed to show error message box HRESULT=%lx\n" ANSI_RESET, res);
	}
	free(wideTitle);
	free(wideMessage);
#else
	// ReSharper disable CppPrintfBadFormat
	char *text_argument = malloc(strlen("--text=") + strlen(message) + 1);
	if (!text_argument)
	{
		printf(ANSI_RED "bootstrap: malloc failed: %s\n" ANSI_RESET, strerror(errno));
		return;
	}
	sprintf(text_argument, "--text=%s", message);
	char *title_argument = malloc(strlen("--title=") + strlen(title) + 1);
	if (!title_argument)
	{
		free(text_argument);
		printf(ANSI_RED "bootstrap: malloc failed: %s\n" ANSI_RESET, strerror(errno));
		return;
	}
	sprintf(title_argument, "--title=%s", title);
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
