//
// Created by droc101 on 1/22/26.
//

#include <actor/LaserEmitter.h>
#include <engine/assets/AssetReader.h>
#include <engine/assets/ModelLoader.h>
#include <engine/helpers/MathEx.h>
#include <engine/structs/Actor.h>
#include <engine/structs/Color.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/Item.h>
#include <engine/structs/Map.h>
#include <item/LaserStopperItem.h>
#include <joltc/Math/Quat.h>
#include <joltc/Math/Vector3.h>
#include <stdbool.h>
#include <wchar.h>

static void LaserStopperItemSwitchFunction(Item *this, Viewmodel *viewmodel)
{
	(void)this;
	viewmodel->enabled = false;
}

static bool LaserStopperItemCanTargetFunction(Item *this, Actor *targetedActor, Color *crosshairColor)
{
	(void)this;
	if (targetedActor->definition->actorType == ACTOR_TYPE_LASER_EMITTER)
	{
		*crosshairColor = CROSSHAIR_COLOR_ENEMY;
		return true;
	}
	return false;
}

static bool LaserStopperItemPrimaryAction(Item *this)
{
	(void)this;
	const GlobalState *state = GetState();
	ActorTriggerInput(NULL, state->map->player.targetedActor, LASER_EMITTER_INPUT_TURN_OFF, &PARAM_NONE);
	// crosshairColor = CROSSHAIR_COLOR_NORMAL;
	return true;
}

const ItemDefinition laserStopperItemDefinition = {
	.name = "Laser Stopper 3000",
	.Construct = DefaultItemConstruct,
	.Destruct = DefaultItemDestruct,

	.FixedUpdate = DefaultItemFixedUpdateFunction,
	.Update = DefaultItemUpdateFunction,
	.RenderHud = DefaultItemHudRenderFunction,

	.CanTarget = LaserStopperItemCanTargetFunction,

	.SwitchTo = LaserStopperItemSwitchFunction,
	.SwitchFrom = DefaultItemSwitchFromFunction,

	.PrimaryActionDown = LaserStopperItemPrimaryAction,
	.PrimaryActionUp = DefaultItemUseFunction,
	.SecondaryActionDown = DefaultItemUseFunction,
	.SecondaryActionUp = DefaultItemUseFunction,
};
