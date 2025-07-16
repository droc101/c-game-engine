//
// Created by droc101 on 11/5/24.
//

#include "Logging.h"
#include <stdio.h>
#include "Error.h"

/// The length of the longest value passed to the type argument of the LogInternal function (including the null) plus 7
#define bufferLength 14

FILE *LogFile = NULL;

void LogInit()
{
	char *folderPath = SDL_GetPrefPath(APPDATA_ORG_NAME, APPDATA_APP_NAME);
	const char *fileName = "game.log";
	char *filePath = malloc(strlen(folderPath) + strlen(fileName) + 1);
	CheckAlloc(filePath);
	strcpy(filePath, folderPath);
	strcat(filePath, fileName);

	SDL_free(folderPath);

	LogFile = fopen(filePath, "w");
	free(filePath);
	if (LogFile == NULL)
	{
		Error("Failed to open log file");
	}
}

void LogDestroy()
{
	if (LogFile != NULL)
	{
		fclose(LogFile);
	}
}

void LogInternal(const char *type, const int color, const bool flush, const char *message, ...)
{
	va_list args;
	va_start(args, message);
	char buf[bufferLength];
	sprintf(buf, "\x1b[%02dm[%s]", color, type);
	printf("%-" TO_STR(bufferLength) "s", buf);
	vprintf(message, args);
	printf("\x1b[0m");
	if (flush)
	{
		fflush(stdout);
	}
	va_end(args);
	va_start(args, message);
	fprintf(LogFile, "[%s] ", type);
	vfprintf(LogFile, message, args);
	va_end(args);
}
