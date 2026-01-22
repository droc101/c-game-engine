//
// Created by droc101 on 1/21/26.
//

#ifndef GAME_ITEM_H
#define GAME_ITEM_H

#include <engine/structs/Actor.h>
#include <engine/structs/Color.h>
#include <engine/structs/Viewmodel.h>
#include <stdbool.h>
#include <stddef.h>

// forward declare to prevent cyclic dep.
typedef struct GlobalState GlobalState;

typedef struct Item Item;
typedef struct ItemDefinition ItemDefinition;

typedef void (*ItemConstructFunction)(Item *this);
typedef void (*ItemDestructFunction)(Item *this);
typedef void (*ItemFixedUpdateFunction)(Item *this, GlobalState *state, double delta);
typedef void (*ItemUpdateFunction)(Item *this, GlobalState *state);
typedef bool (*ItemCanTargetFunction)(Item *this, Actor *targetedActor, Color *crosshairColor);
typedef void (*ItemHudRenderFunction)(Item *this);
typedef void (*ItemSwitchFunction)(Item *this, Viewmodel *viewmodel);
typedef bool (*ItemUseFunction)(Item *this);

struct ItemDefinition
{
	ItemConstructFunction Construct;
	ItemDestructFunction Destruct;

	ItemFixedUpdateFunction FixedUpdate;
	ItemUpdateFunction Update;
	ItemHudRenderFunction RenderHud;

	ItemCanTargetFunction CanTarget;

	ItemSwitchFunction Switch;
	ItemUseFunction PrimaryAction;
	ItemUseFunction SecondaryAction;
};

struct Item
{
	const ItemDefinition *definition;
	void *data;
};

void DefaultItemConstruct(Item *this);

void DefaultItemDestruct(Item *this);

void DefaultItemFixedUpdateFunction(Item *this, GlobalState *state, double delta);

void DefaultItemUpdateFunction(Item *this, GlobalState *state);

bool DefaultItemCanTargetFunction(Item *this, Actor *targetedActor, Color *crosshairColor);

void DefaultItemHudRenderFunction(Item *this);

void DefaultItemSwitchFunction(Item *this, Viewmodel *viewmodel);

bool DefaultItemUseFunction(Item *this);

#endif //GAME_ITEM_H
