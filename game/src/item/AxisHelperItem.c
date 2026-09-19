//
// Created by droc101 on 9/19/26.
//

#include "item/AxisHelperItem.h"
#include <cglm/quat.h>
#include <engine/assets/AssetReader.h>
#include <engine/helpers/MathEx.h>
#include <engine/physics/Physics.h>
#include <engine/structs/Actor.h>
#include <engine/structs/Color.h>
#include <engine/structs/ControlOptions.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/InputAction.h>
#include <engine/structs/Item.h>
#include <engine/structs/KVList.h>
#include <engine/structs/Map.h>
#include <engine/structs/Player.h>
#include <engine/structs/Viewmodel.h>
#include <engine/subsystem/Input.h>
#include <stdbool.h>
#include <wchar.h>
#include "actor/prop/LaserEmitter.h"

static void AxisHelperItemSwitchFunction(Item *this, Viewmodel *viewmodel)
{
	(void)this;
	viewmodel->enabled = true;
	viewmodel->enabled = true;
	viewmodel->model = LoadModel(MODEL("axis_helper"));
	viewmodel->transform.position.x = 0.0f;
	viewmodel->transform.position.y = 0.0f;
	viewmodel->transform.position.z = 4.0f;
}

static void AxisHelperItemUpdateFunction(Item *this, GlobalState *state)
{
	(void)this;

	Viewmodel *viewmodel = &state->map->viewmodel;
	versor rotationQuat;
	QUAT_TO_VERSOR(state->camera->transform.rotation, rotationQuat);
	versor rotationOffset;
	glm_quatv(rotationOffset, GLM_PIf, GLM_XUP);
	glm_quat_mul(rotationQuat, rotationOffset, rotationQuat);
	glm_quat_inv(rotationQuat, rotationQuat);
	VERSOR_TO_QUAT(rotationQuat, viewmodel->transform.rotation);
}

static bool AxisHelperItemCanTargetFunction(Item *this, Actor *targetedActor, Color *crosshairColor, const double delta)
{
	(void)this;
	(void)delta;
	(void)targetedActor;
	*crosshairColor = CROSSHAIR_COLOR_INVISIBLE;
	return false;
}

const ItemDefinition AXIS_HELPER_ITEM_DEFINITION = {
	.name = "Axis Helper",
	.Construct = DefaultItemConstruct,
	.Destruct = DefaultItemDestruct,

	.Update = AxisHelperItemUpdateFunction,
	.RenderHud = DefaultItemHudRenderFunction,
	.FixedUpdate = AxisHelperItemCanTargetFunction,

	.SwitchTo = AxisHelperItemSwitchFunction,
	.SwitchFrom = DefaultItemSwitchFromFunction,
};
