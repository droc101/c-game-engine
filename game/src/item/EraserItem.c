//
// Created by droc101 on 1/21/26.
//

#include <engine/assets/AssetReader.h>
#include <engine/assets/ModelLoader.h>
#include <engine/helpers/MathEx.h>
#include <engine/structs/Actor.h>
#include <engine/structs/Color.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/Item.h>
#include <engine/structs/Map.h>
#include <item/EraserItem.h>
#include <joltc/Math/Quat.h>
#include <joltc/Math/Vector3.h>
#include <stdbool.h>
#include <wchar.h>

static void EraserItemSwitchFunction(Item *this, Viewmodel *viewmodel)
{
	(void)this;
	viewmodel->enabled = true;
	viewmodel->transform.position.x = 0.5f;
	viewmodel->enabled = true;
	viewmodel->model = LoadModel(MODEL("eraser"));
	JPH_Quat_Rotation(&Vector3_AxisY, degToRad(5), &viewmodel->transform.rotation);
}

static bool EraserItemCanTargetFunction(Item *this, Actor *targetedActor, Color *crosshairColor)
{
	(void)this;
	if ((targetedActor->actorFlags & ACTOR_FLAG_ENEMY) == ACTOR_FLAG_ENEMY)
	{
		crosshairColor->r = 0xff;
		crosshairColor->g = 0x00;
		crosshairColor->b = 0x00;
		crosshairColor->a = 0xff;
		return true;
	}
	return false;
}

static bool EraserItemPrimaryAction(Item *this)
{
	(void)this;
	const GlobalState *state = GetState();
	RemoveActor(state->map->player.targetedActor);
	state->map->player.targetedActor = NULL;
	// crosshairColor = COLOR(0xFFFFCCCC);
	return true;
}

const ItemDefinition eraserItemDefinition = {
	.Construct = DefaultItemConstruct,
	.Destruct = DefaultItemDestruct,
	.Switch = EraserItemSwitchFunction,
	.FixedUpdate = DefaultItemFixedUpdateFunction,
	.Update = DefaultItemUpdateFunction,
	.CanTarget = EraserItemCanTargetFunction,
	.RenderHud = DefaultItemHudRenderFunction,
	.PrimaryAction = EraserItemPrimaryAction,
	.SecondaryAction = DefaultItemUseFunction,
};
