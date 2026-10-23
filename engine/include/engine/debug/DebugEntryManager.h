//
// Created by droc101 on 7/19/26.
//

/**
 * @file DebugEntryManager.h
 * Registers and managed debug entries
 */

#ifndef GAME_DEBUGENTRYMANAGER_H
#define GAME_DEBUGENTRYMANAGER_H

#include <engine/structs/KVList.h>
#include <engine/structs/List.h>
#include <stdbool.h>

/// Function signature for a debug entry
typedef void (*DebugEntryFunction)();

/// Visibility mode for a debug entry
typedef enum DebugEntryMode
{
	/// This debug entry will never be visible
	DEBUG_ENTRY_DISABLED,
	/// This debug entry will be shown only in the extended menu
	DEBUG_ENTRY_TOGGLE,
	/// This debug entry will always be shown
	DEBUG_ENTRY_SHOWN,
	/// Number of debug entry modes, not a valid mode
	DEBUG_ENTRY_MODE_MAX,
} DebugEntryMode;

/// Which (if any) side of the screen a debug entry should draw on
typedef enum DebugEntrySide
{
	/// This debug entry will be shown on the left side of the screen
	DEBUG_ENTRY_LEFT_SIDE,
	/// This debug entry will be shown on the right side of the screen
	DEBUG_ENTRY_RIGHT_SIDE,
	/// This debug entry does not allow/support screen side configuration
	DEBUG_ENTRY_NO_SIDE,
} DebugEntrySide;

/// A registered debug entry
typedef struct DebugEntry
{
	/// The key/name of this debug entry
	char *key;
	/// The function to process & draw this debug entry
	DebugEntryFunction process;
	/// The current display mode of this debug entry
	DebugEntryMode mode;
	/// The default display mode of this debug entry
	DebugEntryMode defaultMode;
	/// The current side this debug entry is shown on
	DebugEntrySide side;
	/// The default side this debug entry is shown on
	DebugEntrySide defaultSide;
	/// How much spacing should be added after this debug entry
	int spacing;
} DebugEntry;

/// All registered debug entries
extern List debugEntries;

/**
 * Check if a debug entry is visible
 * @param key The entry to check for
 * @note This function is expensive
 */
bool IsDebugEntryVisible(const char *key);

/**
 * Initialize the debug entry manager
 * @private
 */
void InitDebugEntryManager();

/**
 * Render visible debug entries
 * @private
 */
void RenderDebugEntries();

/**
 * Load the default debug entry settings
 * @private
 */
void DefaultDebugEntrySettings();

/**
 * Load debug entry settings
 * @private
 */
void LoadDebugEntrySettings(KvList from);

/**
 * Save debug entry settings
 * @private
 */
void SaveDebugEntrySettings(KvList to);

/**
 * Destroy the debug entry manager
 * @private
 */
void DestroyDebugEntryManager();

#endif //GAME_DEBUGENTRYMANAGER_H
