//
// Created by droc101 on 1/22/26.
//

#include <actor/LaserEmitter.h>
#include <engine/structs/Actor.h>
#include <engine/structs/Color.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/Item.h>
#include <engine/structs/KVList.h>
#include <engine/structs/Map.h>
#include <engine/structs/Player.h>
#include <engine/subsystem/Input.h>
#include <item/LaserStopperItem.h>
#include <SDL_gamecontroller.h>
#include <SDL_mouse.h>
#include <stdbool.h>
#include <wchar.h>

static void LaserStopperItemSwitchFunction(Item *this, Viewmodel *viewmodel)
{
	(void)this;
	viewmodel->enabled = false;
}

static bool LaserStopperItemCanTargetFunction(Item *this, Actor *targetedActor, Color *crosshairColor, double delta)
{
	(void)this;
	(void)delta;
	if (targetedActor && targetedActor->definition == &laserEmitterActorDefinition)
	{
		*crosshairColor = CROSSHAIR_COLOR_ENEMY;

		if (IsMouseButtonJustPressedPhys(SDL_BUTTON_LEFT) || IsButtonJustPressedPhys(SDL_CONTROLLER_BUTTON_X))
		{
			const GlobalState *state = GetState();
			ActorTriggerInput(NULL, state->map->player.targetedActor, LASER_EMITTER_INPUT_TURN_OFF, &PARAM_NONE);
		} else if (IsMouseButtonJustPressedPhys(SDL_BUTTON_RIGHT) || IsButtonJustPressedPhys(SDL_CONTROLLER_BUTTON_Y))
		{
			const GlobalState *state = GetState();
			ActorTriggerInput(NULL, state->map->player.targetedActor, LASER_EMITTER_INPUT_TURN_ON, &PARAM_NONE);
		}

		return true;
	}
	return false;
}

const ItemDefinition laserStopperItemDefinition = {
	.name = "Laser Stopper 3000",
	.Construct = DefaultItemConstruct,
	.Destruct = DefaultItemDestruct,

	.Update = DefaultItemUpdateFunction,
	.RenderHud = DefaultItemHudRenderFunction,
	.FixedUpdate = LaserStopperItemCanTargetFunction,

	.SwitchTo = LaserStopperItemSwitchFunction,
	.SwitchFrom = DefaultItemSwitchFromFunction,
};
