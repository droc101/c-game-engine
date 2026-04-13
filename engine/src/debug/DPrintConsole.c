//
// Created by droc101 on 2/15/26.
//

#include <engine/debug/DPrint.h>
#include <engine/debug/DPrintConsole.h>
#include <engine/structs/Color.h>
#include <engine/structs/List.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Timing.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef BUILDSTYLE_DEBUG
#include <engine/helpers/Arguments.h>
#endif

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

const Color ansiColors[] = {
	// NORMAL COLORS
	COLOR_BLACK, // BLACK
	COLOR(0xFFD00000), // RED
	COLOR(0xFF00D000), // GREEN
	COLOR(0xFFD0D000), // YELLOW,
	COLOR(0xFF0000D0), // BLUE
	COLOR(0xFFD000D0), // MAGENTA,
	COLOR(0xFF00D0D0), // CYAN,
	COLOR(0xFFe0e0e0), // WHITE,
	COLOR(0), // UNUSED
	COLOR(0xFFe0e0e0), // DEFAULT

	// BRIGHT COLORS
	COLOR(0xFF404040), // BRIGHT BLACK
	COLOR(0xFFFF0000), // BRIGHT RED
	COLOR(0xFF00FF00), // BRIGHT GREEN
	COLOR(0xFFFFFF00), // BRIGHT YELLOW,
	COLOR(0xFF0000FF), // BRIGHT BLUE
	COLOR(0xFFFF00FF), // BRIGHT MAGENTA,
	COLOR(0xFF00FFFF), // BRIGHT CYAN,
	COLOR_WHITE, // BRIGHT WHITE,
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

void AddConsoleMessage(const char *msg, const int color)
{
	if (!consoleEnabled)
	{
		return;
	}
	assert((color >= 30 && color < 40) || (color >= 90 && color < 98));
	ConsoleMessage *cm = malloc(sizeof(ConsoleMessage));
	CheckAlloc(cm);
	cm->message = strdup(msg);
	if (color >= 90)
	{
		cm->color = ansiColors[(color % 10) + 9];
	} else
	{
		cm->color = ansiColors[color % 10];
	}
	// time will be set when the message is first processed so a 2-second frame doesn't result in messages not getting shown
	cm->time = 0;
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
