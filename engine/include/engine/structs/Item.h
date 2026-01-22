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
	/// The display name of this item type
	const char *name;

	/// Called when the player is given this item
	ItemConstructFunction Construct;
	/// TODO: call this when global state is being destroyed
	ItemDestructFunction Destruct;

	/// Called once per tick when this item is active
	ItemFixedUpdateFunction FixedUpdate;
	/// Called once per frame when this item is active
	ItemUpdateFunction Update;
	/// Called once per frame when this item is active, during HUD drawing
	ItemHudRenderFunction RenderHud;

	/// Called to check if this item can handle the targeted actor. Set the crosshair color if it can.
	ItemCanTargetFunction CanTarget;

	/// Called when the player switches to this item (including on map load)
	ItemSwitchFunction SwitchTo;
	/// Called when the player switches away from this item
	ItemSwitchFunction SwitchFrom;

	/// Called when the player begins to press the primary action (lclick)
	ItemUseFunction PrimaryActionDown;
	/// Called when the player releases the primary action (does NOT get called if the item is switched away during action)
	ItemUseFunction PrimaryActionUp;
	/// Called when the player begins to press the secondary action (rclick)
	ItemUseFunction SecondaryActionDown;
	/// Called when the player releases the secondary action (does NOT get called if the item is switched away during action)
	ItemUseFunction SecondaryActionUp;
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

void DefaultItemSwitchToFunction(Item *this, Viewmodel *viewmodel);

void DefaultItemSwitchFromFunction(Item *this, Viewmodel *viewmodel);

bool DefaultItemUseFunction(Item *this);

#endif //GAME_ITEM_H
