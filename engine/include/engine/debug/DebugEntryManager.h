//
// Created by droc101 on 7/19/26.
//

#ifndef GAME_DEBUGENTRYMANAGER_H
#define GAME_DEBUGENTRYMANAGER_H

#include <engine/structs/List.h>
#include <stdbool.h>

typedef void (*DebugEntryFunction)();

typedef struct DebugEntry
{
	char *key;
	DebugEntryFunction process;
	bool enabled;
	bool defaultEnabled;
	int spacing;
} DebugEntry;

extern List debugEntries;

void InitDebugEntryManager();

void RenderDebugEntries();

void SaveDebugEntrySettings();

void DestroyDebugEntryManager();

#endif //GAME_DEBUGENTRYMANAGER_H
