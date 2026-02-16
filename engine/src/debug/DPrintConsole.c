//
// Created by droc101 on 2/15/26.
//

#include <engine/debug/DPrint.h>
#include <engine/debug/DPrintConsole.h>
#include <engine/helpers/Arguments.h>
#include <engine/structs/Color.h>
#include <engine/structs/List.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Timing.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define CONSOLE_MESSAGE_VISIBLE_FOR_MS 2000

static bool consoleEnabled = false;
static List consoleMessages;

typedef struct ConsoleMessage ConsoleMessage;

struct ConsoleMessage
{
	char *message;
	Color color;
	size_t time;
};

void InitDPrintConsole()
{
#ifndef BUILDSTYLE_DEBUG
	if (HasCliArg("--show-console"))
#endif
	{
		consoleEnabled = true;
		ListInit(consoleMessages, LIST_POINTER);
	}
}

void DestroyDPrintConsole()
{
	if (consoleEnabled)
	{
		consoleEnabled = false;
		for (size_t i = 0; i < consoleMessages.length; i++)
		{
			const ConsoleMessage *msg = ListGetPointer(consoleMessages, i);
			free(msg->message);
		}
		ListAndContentsFree(consoleMessages);
	}
}

void AddConsoleMessage(const char *msg, const Color color)
{
	if (!consoleEnabled)
	{
		return;
	}
	ConsoleMessage *cm = malloc(sizeof(ConsoleMessage));
	CheckAlloc(cm);
	cm->message = strdup(msg);
	cm->color = color;
	cm->time = 0; // time will be set when the message is first processed so a 2 second frame doesn't result in messages not getting shown
	ListAdd(consoleMessages, cm);
}

void ProcessDPrintConsole()
{
	size_t indexToRemove = SIZE_MAX;
	for (size_t i = 0; i < consoleMessages.length; i++)
	{
		ConsoleMessage *msg = ListGetPointer(consoleMessages, i);
		DPrint(msg->message, msg->color);
		if (msg->time == 0)
		{
			msg->time = GetTimeMs();
		} else if (indexToRemove == SIZE_MAX && GetTimeMs() > msg->time + CONSOLE_MESSAGE_VISIBLE_FOR_MS)
		{
			indexToRemove = i;
		}
	}
	if (indexToRemove != SIZE_MAX)
	{
		ConsoleMessage *msg = ListGetPointer(consoleMessages, indexToRemove);
		free(msg->message);
		free(msg);
		ListRemoveAt(consoleMessages, indexToRemove);
	}
}
