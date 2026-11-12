//
// Created by droc101 on 1/21/26.
//

/**
 * @file Item.h
 * Default item definition functions
 */

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

/**
 * Function signature for constructing a new item
 * @param this Item to construct
 */
typedef void (*ItemConstructFunction)(Item *this);
/**
 * Function signature for destructing an item
 * @param this The item to destruct
 */
typedef void (*ItemDestructFunction)(Item *this);
/**
 * Function signature for frame updating an item
 * @param this The item to update
 * @param state GlobalState
 */
typedef void (*ItemUpdateFunction)(Item *this, GlobalState *state);
/**
 * Function signature for physics updating an item
 * @param this The item to update
 * @param targetedActor The actor the player is looking at, or @c NULL
 * @param crosshairColor The player's crosshair color
 * @param delta Physics delta
 */
typedef bool (*ItemFixedUpdateFunction)(Item *this, Actor *targetedActor, Color *crosshairColor, double delta);
/**
 * Function signature for rendering an item's HUD
 * @param this The item to render HUD for
 */
typedef void (*ItemHudRenderFunction)(Item *this);
/**
 * Function signature for when a player switches to an item
 * @param this The item that was switched to
 * @param viewmodel The viewmodel
 */
typedef void (*ItemSwitchFunction)(Item *this, Viewmodel *viewmodel);

/// An item class definition
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

/// An item in the player's inventory
struct Item
{
	/// The definition of this item
	const ItemDefinition *definition;
	/// Extra data pointer for item-specific data
	void *data;
};

/// Default item construct function
void DefaultItemConstruct(Item *);

/// Default item destruct function
void DefaultItemDestruct(Item *);

/// Default item update function
void DefaultItemUpdateFunction(Item *, GlobalState *);

/// Default item can target function
bool DefaultItemCanTargetFunction(Item *, Actor *, Color *, double);

/// Default item HUD render function
void DefaultItemHudRenderFunction(Item *);

/// Default item switch to function
void DefaultItemSwitchToFunction(Item *, Viewmodel *);

/// Default item switch from function
void DefaultItemSwitchFromFunction(Item *, Viewmodel *);

#endif //GAME_ITEM_H
