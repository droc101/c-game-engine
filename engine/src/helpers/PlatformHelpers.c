//
// Created by droc101 on 11/10/2024.
//

#include <engine/helpers/PlatformHelpers.h>
#include <engine/structs/GlobalState.h>
#include <engine/subsystem/Logging.h>
#include <SDL3/SDL_video.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef WIN32
#include <ctype.h>
#include <dwmapi.h>
#include <handleapi.h>
#include <minwindef.h>
#include <processthreadsapi.h>
#include <SDL3/SDL_properties.h>
#include <SDL3/SDL_video.h>
#include <string.h>
#include <winbase.h>
#endif

void SetDwmWindowAttribs(SDL_Window *window)
{
#ifdef WIN32

	const HWND hWnd = (HWND)SDL_GetPointerProperty(SDL_GetWindowProperties(window),
												   SDL_PROP_WINDOW_WIN32_HWND_POINTER,
												   NULL);
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
	const bool lengthCheck = strlen(path) >= 3;
	const bool letterCheck = isalpha(path[0]);
	const bool colonCheck = path[1] == ':';
	const bool slashCheck = path[2] == '/' || path[2] == '\\';
	if (lengthCheck && letterCheck && colonCheck && slashCheck)
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

const char *GameStrCaseStr(const char *haystack, const char *needle)
{
	if (!haystack)
	{
		return NULL;
	}
	if (!needle)
	{
		return haystack;
	}

	const size_t haystackLen = strlen(haystack);
	const size_t needleLen = strlen(needle);

	if (needleLen == 0)
	{
		return haystack;
	}
	if (needleLen > haystackLen)
	{
		return NULL;
	}

	for (size_t i = 0; i < haystackLen - needleLen; i++)
	{
		const char *base = &haystack[i];
		bool match = true;
		for (size_t j = 0; j < needleLen; j++)
		{
			if (tolower(base[j]) != tolower(needle[j]))
			{
				match = false;
				break;
			}
		}
		if (match)
		{
			return base;
		}
	}

	return NULL;
}
char *CanonicalFilePath(const char *path)
{
#ifdef WIN32
	char *buffer = malloc(MAX_PATH);
	CheckAlloc(buffer);
	const DWORD result = GetFullPathName(path, MAX_PATH, buffer, NULL);
	if (result == 0)
	{
		free(buffer);
		return NULL;
	}

	// windows why do you have to be special
	for (size_t i = 0; i < result; i++)
	{
		if (buffer[i] == '\\')
		{
			buffer[i] = '/';
		}
	}

	return buffer;
#else
	return realpath(path, NULL);
#endif
}

bool RedirectFd(const int originalFd, int *pipeFds, int *originalFdCopy)
{
#ifdef SDL_PLATFORM_LINUX
	*originalFdCopy = dup(originalFd);
	if (pipe(pipeFds) != 0)
	{
		return false;
	}
	dup2(pipeFds[1], originalFd);
	close(pipeFds[1]);
	return true;
#else
	return false;
#endif
}

void RestoreFd(const int modifiedFd, int *pipeFds, const int originalFd)
{
#ifdef SDL_PLATFORM_LINUX
	dup2(originalFd, modifiedFd);
	close(pipeFds[0]);
#endif
}
