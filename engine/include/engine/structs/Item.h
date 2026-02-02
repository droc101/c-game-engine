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
typedef void (*ItemUpdateFunction)(Item *this, GlobalState *state);
typedef bool (*ItemFixedUpdateFunction)(Item *this, Actor *targetedActor, Color *crosshairColor, double delta);
typedef void (*ItemHudRenderFunction)(Item *this);
typedef void (*ItemSwitchFunction)(Item *this, Viewmodel *viewmodel);

struct ItemDefinition
{
	/// The display name of this item type
	const char *name;

	/// Called when the player is given this item
	ItemConstructFunction Construct;
	/// Called when global state is being destroyed
	ItemDestructFunction Destruct;

	/// Called once per frame when this item is active
	ItemUpdateFunction Update;
	/// Called once per frame when this item is active, during HUD drawing
	ItemHudRenderFunction RenderHud;
	/// Called every tick to handle targeting, using, and crosshair color.
	ItemFixedUpdateFunction FixedUpdate;

	/// Called when the player switches to this item (including on map load)
	ItemSwitchFunction SwitchTo;
	/// Called when the player switches away from this item
	ItemSwitchFunction SwitchFrom;
};

struct Item
{
	const ItemDefinition *definition;
	void *data;
};

void DefaultItemConstruct(Item *this);

void DefaultItemDestruct(Item *this);

void DefaultItemUpdateFunction(Item *this, GlobalState *state);

bool DefaultItemCanTargetFunction(Item *this, Actor *targetedActor, Color *crosshairColor, double delta);

void DefaultItemHudRenderFunction(Item *this);

void DefaultItemSwitchToFunction(Item *this, Viewmodel *viewmodel);

void DefaultItemSwitchFromFunction(Item *this, Viewmodel *viewmodel);

#endif //GAME_ITEM_H
