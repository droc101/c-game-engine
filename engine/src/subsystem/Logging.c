//
// Created by droc101 on 11/5/24.
//

#include <engine/debug/DPrintConsole.h>
#include <engine/Engine.h>
#include <engine/structs/Color.h>
#include <engine/structs/GlobalState.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Logging.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/// The length of the longest value passed to the type argument of the LogInternal function (including the null) plus 7
#define bufferLength 14

FILE *logFile = NULL;

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
		LogError("Failed to open log file");
	}
}

void LogDestroy()
{
	LogDebug("Cleaning up logging system...goodbye!\n");
	if (logFile != NULL)
	{
		fclose(logFile);
	}
}

void LogInternal(const char *type, const int color, const bool flush, const char *message, ...)
{
	va_list args;
	va_start(args, message);
	char buf[bufferLength];
	size_t length = 0;
	if (!type)
	{
		length = sprintf(buf, "\x1b[%02dm", color);
	} else
	{
		length = sprintf(buf, "\x1b[%02dm[%s]", color, type);
	}
	printf("%-" TO_STR(bufferLength) "s", buf);
	length += vprintf(message, args);
	printf("\x1b[0m");
	va_end(args);

	va_start(args, message);
	char *plainTextBuffer = calloc(sizeof(char), length + 1);
	CheckAlloc(plainTextBuffer);
	sprintf(plainTextBuffer,
			"[%.*s] ",
			bufferLength - 8,
			type); // Minus 8 due to color, brackets, and null not included
	vsprintf(plainTextBuffer + strlen(plainTextBuffer), message, args);
	AddConsoleMessage(plainTextBuffer, COLOR_WHITE);

	if (logFile)
	{
		fprintf(logFile, "%s", plainTextBuffer);
		// vfprintf(logFile, message, args);
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
