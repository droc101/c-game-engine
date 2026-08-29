//
// Created by droc101 on 7/26/26.
//

#ifndef GAME_CONTROLOPTIONS_H
#define GAME_CONTROLOPTIONS_H

#include <engine/structs/InputAction.h>
#include <engine/structs/KVList.h>
#include <engine/structs/List.h>
#include <stdbool.h>

typedef struct ControlCategory
{
	char key[128];
	char categoryName[128];
	List controlOptions;
} ControlCategory;

typedef struct ControlOption
{
	char key[128];
	char displayName[128];
	InputAction *action;
	bool allowAxisBind;
	const InputAction *defaultAction;
} ControlOption;

typedef void (*ControlRegisterFunction)(void);

extern List controlCategories;

#pragma region Engine Input Actions

extern InputAction moveForward;
extern InputAction moveBackward;
extern InputAction moveLeft;
extern InputAction moveRight;
extern InputAction sprint;
extern InputAction sneak;
extern InputAction jump;

extern InputAction interact;
extern InputAction primaryAttack;
extern InputAction secondaryAttack;
extern InputAction previousItem;
extern InputAction nextItem;

extern InputAction lookUp;
extern InputAction lookDown;
extern InputAction lookLeft;
extern InputAction lookRight;

extern InputAction debugMenu;
extern InputAction noclip;
extern InputAction freecam;
extern InputAction benchmark;

extern InputAction fullscreen;

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
 */
void RegisterControls(ControlRegisterFunction RegisterGameControls);

/**
 * Reset all control bindings to the default
 */
void DefaultControls();

/**
 * Load control options
 */
void LoadControls(KvList from);

/**
 * Save control options
 */
void SaveControls(KvList to);

/**
 * Destroy controls system
 */
void DestroyControls();

#endif //GAME_CONTROLOPTIONS_H
