//
// Created by droc101 on 11/10/2024.
//

#include <engine/helpers/Arguments.h>
#include <engine/helpers/PlatformHelpers.h>
#include <engine/structs/GlobalState.h>
#include <engine/subsystem/Logging.h>
#include <errno.h>
#include <SDL3/SDL_video.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef WIN32
// clang-format off
#include <windows.h>
#include <ctype.h>
#include <handleapi.h>
#include <minwindef.h>
#include <processthreadsapi.h>
#include <SDL3/SDL_properties.h>
#include <SDL3/SDL_video.h>
#include <string.h>
#include <winbase.h>
#include <dbghelp.h>
// clang-format on
#else
#include <execinfo.h>
#include <signal.h>
#include <sys/stat.h>
#endif

_Noreturn void RestartProgram()
{
	LogWarning("Exiting early to restart engine, resources may not get cleaned properly.\n"); // TODO clean properly
#ifdef WIN32
	STARTUPINFO si = {0};
	PROCESS_INFORMATION pi = {0};
	si.cb = sizeof(si);
	CreateProcess(
			GetState()->executablePath,
			GetCommandLine(),
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
	execv(GetState()->executablePath, GetArgv());
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

void OpenFileInDefaultProgram(const char *filePath)
{
#ifdef WIN32
	ShellExecuteA(NULL, "open", filePath, NULL, NULL, 0);
#else
	const pid_t pid = fork();
	if (pid == -1)
	{
		LogError("fork() failed: %s\n", strerror(errno));
		return;
	}
	if (pid == 0) // 0 = child
	{
		char *argv[] = {
			"xdg-open",
			strdup(filePath),
			NULL,
		};
		if (execvp("xdg-open", argv) == -1)
		{
			LogError("execvp() failed: %s\n", strerror(errno));
		}
	}
#endif
}

void FixupPath(char *path)
{
#ifdef WIN32
	for (size_t i = 0; i < strlen(path); i++)
	{
		if (path[i] == '\\')
		{
			path[i] = '/';
		}
	}
#else
	(void)path;
#endif
}

bool MakeDirectory(const char *path)
{
#ifdef WIN32
	return mkdir(path) != 0;
#else
	return mkdir(path, 0660) != 0;
#endif
}

void RaiseDebugger()
{
#ifdef WIN32
	__debugbreak(); // SIGTRAP doesn't exist on Windows
#else
	// emit sigtrap to allow debugger to catch the error
	raise(SIGTRAP);
#endif
}

void PrintStackTrace()
{
#ifdef WIN32
	void *frames[512];
	const HANDLE hProcess = GetCurrentProcess();
	SymInitialize(hProcess, NULL, TRUE);
	SYMBOL_INFO *symbol = calloc(1, sizeof(SYMBOL_INFO) + 256);
	// not using CheckAlloc here because this may be run in memory starved conditions, nullness will be checked before use
	if (symbol)
	{
		symbol->MaxNameLen = 255;
		symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
	}
	const uint16_t num_frames = CaptureStackBackTrace(0, 512, frames, NULL);
	if (num_frames > 0)
	{
		LogInfo("Stack Trace:\n");
		for (int i = 0; i < num_frames; i++)
		{
			if (symbol && SymFromAddr(hProcess, (uint64_t)frames[i], 0, symbol))
			{
				LogInfo("    %d: %s+0x%zx [%p]\n", i, symbol->Name, frames[i] - symbol->Address, frames[i]);
			} else
			{
				LogInfo("    %d: ??? [%p]\n", i, frames[i]);
			}
		}
	} else
	{
		LogWarning("Stack trace contained no frames!\n");
	}
	free(symbol);
#else
	void *frames[512];
	const int num_frames = backtrace(frames, 512);
	char **symbols = backtrace_symbols(frames, num_frames);
	if (num_frames > 0)
	{
		LogInfo("Stack Trace:\n");
		if (symbols)
		{
			for (int i = 0; i < num_frames; i++)
			{
				LogInfo("    %d: %s\n", i, symbols[i]);
			}
			free(symbols);
		} else
		{
			LogWarning("Failed to get symbols for stack trace\n");
			for (int i = 0; i < num_frames; i++)
			{
				LogInfo("    %d: %p\n", i, frames[i]);
			}
		}
	} else
	{
		LogWarning("Stack trace contained no frames!\n");
	}
#endif
}
