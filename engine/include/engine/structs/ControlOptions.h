//
// Created by droc101 on 7/26/26.
//

/**
 * @file ControlOptions.h
 * Control option registration & built-in controls
 */

#ifndef GAME_CONTROLOPTIONS_H
#define GAME_CONTROLOPTIONS_H

#include <engine/structs/InputAction.h>
#include <engine/structs/KVList.h>
#include <engine/structs/List.h>
#include <stdbool.h>

/// Control category definition
typedef struct ControlCategory
{
	/// The internal key of this category
	char key[128];
	/// The display name of this category
	char categoryName[128];
	/// The control options in this category
	List controlOptions;
} ControlCategory;

/// Control option definition
typedef struct ControlOption
{
	/// The internal key of this option
	char key[128];
	/// The display name of this option
	char displayName[128];
	/// The current value of this option
	InputAction *action;
	/// Whether to allow binding analog and analog-ish controls to this option
	bool allowAxisBind;
	/// The default binding
	const InputAction *defaultAction;
} ControlOption;

/// Function signature for game-side control registration function
typedef void (*ControlRegisterFunction)(void);

/// All registered control categories
extern List controlCategories;

#pragma region Engine Input Actions

/// Move forward input action
extern InputAction moveForward;
/// Move backward input action
extern InputAction moveBackward;
/// Move left input action
extern InputAction moveLeft;
/// Move right input action
extern InputAction moveRight;
/// Sprint input action
extern InputAction sprint;
/// Sneak/crouch input action
extern InputAction sneak;
/// Jump input action
extern InputAction jump;

/// Interact/pick up input action
extern InputAction interact;
/// Item primary attack input action
extern InputAction primaryAttack;
/// Item secondary attack input action
extern InputAction secondaryAttack;
/// Previous item input action
extern InputAction previousItem;
/// Next item input action
extern InputAction nextItem;

/// Look up input action
extern InputAction lookUp;
/// Look down input action
extern InputAction lookDown;
/// Look left input action
extern InputAction lookLeft;
/// Look right input action
extern InputAction lookRight;

/// Toggle extended debug menu input action
extern InputAction debugMenu;
/// Toggle noclip input action
extern InputAction noclip;
/// Toggle freecam input action
extern InputAction freecam;
/// Start/stop benchmark input action
extern InputAction benchmark;
/// Reload shaders input action
extern InputAction reloadShaders;

/// Toggle fullscreen input action
extern InputAction fullscreen;
/// Toggle flashlight input action
extern InputAction flashlight;

#pragma endregion

/**
 * Get an existing control category, or NULL if it doesn't exist
 * @param key The key to get
 */
ControlCategory *GetControlCategory(const char *key);

/**
 * Register a control category, or get an existing one if the key already exists
 * @param displayName The display name to register
 * @param key The key to register
 */
ControlCategory *RegisterControlCategory(char *displayName, char *key);

/**
 * Register a control binding
 * @param key The key to register
 * @param displayName The display name of this control
 * @param action The input action this control is associated with
 * @param allowAxisBind Whether to allow analog bindings
 * @param defaultAction The default binding
 * @param category The category this control is in
 */
void RegisterControl(char *key,
					 char *displayName,
					 InputAction *action,
					 bool allowAxisBind,
					 const InputAction *defaultAction,
					 ControlCategory *category);

/**
 * Register control bindings
 * @param RegisterGameControls Function to call to register game control bindings
 * @private
 */
void RegisterControls(ControlRegisterFunction RegisterGameControls);

/**
 * Reset all control bindings to the default
 * @private
 */
void DefaultControls();

/**
 * Load control options
 * @private
 */
void LoadControls(KvList from);

/**
 * Save control options
 * @private
 */
void SaveControls(KvList to);

/**
 * Destroy controls system
 * @private
 */
void DestroyControls();

#endif //GAME_CONTROLOPTIONS_H
