//
// Created by droc101 on 7/19/26.
//

#ifndef GAME_DEBUGENTRYMANAGER_H
#define GAME_DEBUGENTRYMANAGER_H

#include <engine/structs/List.h>

typedef void (*DebugEntryFunction)();

typedef enum DebugEntryMode
{
	DEBUG_ENTRY_DISABLED,
	DEBUG_ENTRY_TOGGLE,
	DEBUG_ENTRY_SHOWN,
	DEBUG_ENTRY_MODE_MAX,
} DebugEntryMode;

typedef struct DebugEntry
{
	char *key;
	DebugEntryFunction process;
	DebugEntryMode mode;
	DebugEntryMode defaultMode;
	int spacing;
} DebugEntry;

extern List debugEntries;

void InitDebugEntryManager();

void RenderDebugEntries();

void SaveDebugEntrySettings();

void DestroyDebugEntryManager();

#endif //GAME_DEBUGENTRYMANAGER_H
