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

ControlCategory *GetControlCategory(const char *key);

ControlCategory *RegisterControlCategory(char *displayName, char *key);

void RegisterControl(char *key,
					 char *displayName,
					 InputAction *action,
					 bool allowAxisBind,
					 const InputAction *defaultAction,
					 ControlCategory *category);

void RegisterControls(ControlRegisterFunction RegisterGameControls);

void DefaultControls();

void LoadControls(KvList from);

void SaveControls(KvList to);

void DestroyControls();

#endif //GAME_CONTROLOPTIONS_H
