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
#include <engine/structs/Player.h>
#include <engine/structs/Viewmodel.h>
#include <engine/subsystem/Input.h>
#include <item/EraserItem.h>
#include <joltc/Math/Quat.h>
#include <joltc/Math/Vector3.h>
#include <stdbool.h>
#include <stddef.h>

static void EraserItemSwitchFunction(Item *this, Viewmodel *viewmodel)
{
	(void)this;
	viewmodel->enabled = true;
	viewmodel->transform.position.x = 0.5f;
	viewmodel->enabled = true;
	viewmodel->model = LoadModel(MODEL("eraser"));
	JPH_Quat_Rotation(&Vector3_AxisY, degToRad(5), &viewmodel->transform.rotation);
}

static bool EraserItemCanTargetFunction(Item * /*this*/,
										Actor *targetedActor,
										Color *crosshairColor,
										const double /*delta*/)
{
	if (targetedActor && (targetedActor->flags & ACTOR_FLAG_ENEMY) == ACTOR_FLAG_ENEMY)
	{
		*crosshairColor = CROSSHAIR_COLOR_ENEMY;

		if (IsInputActionJustPressed(physicsThreadInput, &GetState()->options.primaryAttack))
		{
			const GlobalState *state = GetState();
			RemoveActor(state->map->player.targetedActor);
		}

		return true;
	}
	return false;
}

const ItemDefinition ERASER_ITEM_DEFINITION = {
	.name = "Eraser",
	.Construct = DefaultItemConstruct,
	.Destruct = DefaultItemDestruct,

	.Update = DefaultItemUpdateFunction,
	.RenderHud = DefaultItemHudRenderFunction,
	.FixedUpdate = EraserItemCanTargetFunction,

	.SwitchTo = EraserItemSwitchFunction,
	.SwitchFrom = DefaultItemSwitchFromFunction,
};
