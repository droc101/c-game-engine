//
// Created by droc101 on 11/5/24.
//

#include <engine/debug/DPrintConsole.h>
#include <engine/Engine.h>
#include <engine/structs/GlobalState.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Logging.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef SDL_PLATFORM_LINUX
#include <engine/helpers/PlatformHelpers.h>
#include <SDL3/SDL_thread.h>
#include <SDL3/SDL_timer.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <unistd.h>
#endif

/// The length of the longest value passed to the type argument of the LogInternal function (including the null) plus 7
#define BUFFER_LENGTH 14

static FILE *logFile = NULL;

#ifdef SDL_PLATFORM_LINUX
/// Number of bytes the logging thread can read from stdout/err at a time
#define STDIO_BUFFER_SIZE 8192

/// Original stdout file descriptor
static int standardOutFd = STDOUT_FILENO;
/// Pipe taking the place of stdout, [0] is the read end, [1] is the write end
static int standardOutPipe[2];

/// Original stderr file descriptor
static int standardErrorFd = STDERR_FILENO;
/// Pipe taking the place of stderr, [0] is the read end, [1] is the write end
static int standardErrorPipe[2];

static SDL_Thread *loggingThread;
static bool loggingThreadQuit = false;

static int64_t ProcessPipe(const int inFd, const int outFd)
{
	int count = 0;
	ioctl(inFd, FIONREAD, &count);
	if (count < 1)
	{
		return 0;
	}

	static char buffer[STDIO_BUFFER_SIZE];

	const int64_t readSize = read(inFd, buffer, STDIO_BUFFER_SIZE);
	if (readSize > 0)
	{
		write(outFd, buffer, readSize);
		if (logFile)
		{
			buffer[readSize] = '\0';
			fprintf(logFile, "%s", buffer);
		}
	}
	return readSize;
}

static int LogThread(void * /*data*/)
{
	while (!loggingThreadQuit)
	{
		(void)ProcessPipe(standardOutPipe[0], standardOutFd);
		(void)ProcessPipe(standardErrorPipe[0], standardErrorFd);
		SDL_Delay(1);
	}

	// Make sure anything not yet read gets processed before exiting
	int64_t readSize = 0;
	do
	{
		readSize = 0;
		readSize += ProcessPipe(standardOutPipe[0], standardOutFd);
		readSize += ProcessPipe(standardErrorPipe[0], standardErrorFd);
	} while (readSize != 0);

	return 0;
}
#endif

void LogInit()
{
	const char *folderPath = GetState()->executableFolder;
	const char *fileName = "game.log";
	char *filePath = malloc(strlen(folderPath) + strlen(fileName) + 1);
	CheckAlloc(filePath);
	strcpy(filePath, folderPath);
	strcat(filePath, fileName);

	logFile = fopen(filePath, "w");
	free(filePath);
	if (logFile == NULL)
	{
		LogError("Failed to open log file\n");
	}

#ifdef SDL_PLATFORM_LINUX
	if (!RedirectFd(STDOUT_FILENO, standardOutPipe, &standardOutFd))
	{
		Error("Failed to redirect stdout to pipe");
	}
	if (!RedirectFd(STDERR_FILENO, standardErrorPipe, &standardErrorFd))
	{
		Error("Failed to redirect stderr to pipe");
	}

	loggingThread = SDL_CreateThread(LogThread, "GameLoggingThread", NULL);
#endif
}

void LogDestroy()
{
	LogDebug("Cleaning up logging system...goodbye!\n");
#ifdef SDL_PLATFORM_LINUX
	loggingThreadQuit = true;
	SDL_WaitThread(loggingThread, NULL);
	RestoreFd(STDOUT_FILENO, standardOutPipe, standardOutFd);
	RestoreFd(STDERR_FILENO, standardErrorPipe, standardErrorFd);
	if (logFile != NULL)
	{
		fclose(logFile);
		logFile = NULL;
	}
#endif
}

void LogInternal(const char *type, const int color, const bool flush, const char *message, ...)
{
	va_list args;
	va_start(args, message);
	char buf[BUFFER_LENGTH];
	size_t length = 0;
	if (!type)
	{
		length = sprintf(buf, "\x1b[%02dm", color);
	} else
	{
		length = sprintf(buf, "\x1b[%02dm[%s]", color, type);
	}
#ifdef SDL_PLATFORM_LINUX
	dprintf(standardOutFd, "%-" TO_STR(BUFFER_LENGTH) "s", buf);
	length += vdprintf(standardOutFd, message, args);
	dprintf(standardOutFd, "\x1b[0m");
#else
	printf("%-" TO_STR(BUFFER_LENGTH) "s", buf);
	length += vprintf(message, args);
	printf("\x1b[0m");
#endif
	va_end(args);

	va_start(args, message);
	char *plainTextBuffer = calloc(length + 1, sizeof(char));
	CheckAlloc(plainTextBuffer);
	if (type)
	{
		sprintf(plainTextBuffer,
				"[%.*s] ",
				BUFFER_LENGTH - 8,
				type); // Minus 8 due to color, brackets, and null not included
	}
	vsprintf(plainTextBuffer + strlen(plainTextBuffer), message, args);
	AddConsoleMessage(plainTextBuffer, color);

	if (logFile)
	{
		fprintf(logFile, "%s", plainTextBuffer);
	}

	free(plainTextBuffer);
	va_end(args);

	if (flush)
	{
		fflush(stdout);
		if (logFile)
		{
			fflush(logFile);
		}
	}
}
