//
// Created by droc101 on 11/5/24.
//

#include "Logging.h"
#include <stdio.h>
#include "Error.h"

/// The length of the longest value passed to the type argument of the LogInternal function (including the null) plus 7
#define bufferLength 14

FILE *logFile = NULL;

void LogInit()
{
	char *folderPath = SDL_GetPrefPath(APPDATA_ORG_NAME, APPDATA_APP_NAME);
	const char *fileName = "game.log";
	char *filePath = malloc(strlen(folderPath) + strlen(fileName) + 1);
	CheckAlloc(filePath);
	strcpy(filePath, folderPath);
	strcat(filePath, fileName);

	SDL_free(folderPath);

	logFile = fopen(filePath, "w");
	free(filePath);
	if (logFile == NULL)
	{
		Error("Failed to open log file");
	}
}

void LogDestroy()
{
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
	if (!type)
	{
		sprintf(buf, "\x1b[%02dm", color);
	} else
	{
		sprintf(buf, "\x1b[%02dm[%s]", color, type);
	}
	printf("%-" TO_STR(bufferLength) "s", buf);
	vprintf(message, args);
	printf("\x1b[0m");
	va_end(args);
	va_start(args, message);
	fprintf(logFile, "[%.*s] ", bufferLength - 8, type); // Minus 8 due to color, brackets, and null not included
	vfprintf(logFile, message, args);
	va_end(args);
	if (flush)
	{
		fflush(stdout);
		fflush(logFile);
	}
}
