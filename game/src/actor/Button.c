//
// Created by droc101 on 3/6/26.
//

#include "actor/Button.h"
#include <engine/assets/AssetReader.h>
#include <engine/assets/ModelLoader.h>
#include <engine/physics/Physics.h>
#include <engine/structs/Actor.h>
#include <engine/structs/ActorDefinition.h>
#include <engine/structs/KVList.h>
#include <engine/subsystem/Timing.h>
#include <joltc/enums.h>
#include <joltc/Math/Transform.h>
#include <joltc/Physics/Body/BodyCreationSettings.h>
#include <joltc/Physics/Body/BodyInterface.h>
#include <joltc/Physics/Collision/Shape/Shape.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct ButtonData
{
	uint32_t offSkin;
	uint32_t onSkin;
	bool toggleMode; // press e again to turn off
	bool pressed;
	int resetDelay;
	uint64_t timePressed;
} ButtonData;

static inline void CreateButtonCollider(Actor *this, const Transform *transform, JPH_Shape *shape)
{
	JPH_BodyCreationSettings *bodyCreationSettings = JPH_BodyCreationSettings_Create2_GAME(shape,
																						   transform,
																						   JPH_MotionType_Static,
																						   OBJECT_LAYER_STATIC,
																						   this);
	this->bodyId = JPH_BodyInterface_CreateAndAddBody(this->bodyInterface,
													  bodyCreationSettings,
													  JPH_Activation_Activate);
	JPH_BodyCreationSettings_Destroy(bodyCreationSettings);
}

static void ButtonUpdate(Actor *this, const double /*delta*/)
{
	ButtonData *data = this->extraData;
	if (data->pressed && data->resetDelay != -1)
	{
		const size_t timeDiff = GetTimeMs() - data->timePressed;
		if (timeDiff >= (size_t)data->resetDelay)
		{
			data->pressed = false;
			this->currentSkinIndex = data->offSkin;
			ActorFireOutput(this, "unpressed", PARAM_NONE);
		}
	}
}

void ButtonInit(Actor *this, const KvList params, Transform *transform)
{
	this->flags = ACTOR_FLAG_INTERACTABLE;
	this->hasModel = true;
	this->model = LoadModel(KvGetString(params, "model", MODEL("wall_button")));
	JPH_Shape *shape = this->model->boundingBoxShape;
	if (this->model->collisionModelType != COLLISION_MODEL_TYPE_NONE)
	{
		shape = this->model->collisionModelShape;
	}
	CreateButtonCollider(this, transform, shape);
	ButtonData *data = malloc(sizeof(ButtonData));
	this->extraData = data;
	data->offSkin = KvGetInt(params, "off_skin", 0);
	data->onSkin = KvGetInt(params, "on_skin", 1);
	data->pressed = KvGetBool(params, "start_pressed", false);
	data->resetDelay = KvGetInt(params, "reset_delay_ms", -1);
	if (data->pressed && data->resetDelay != -1)
	{
		data->timePressed = GetTimeMs();
	}
	data->toggleMode = KvGetBool(params, "toggle_mode", false);
	this->currentSkinIndex = data->pressed ? data->onSkin : data->offSkin;
}

void ButtonInteract(Actor *this)
{
	ButtonData *data = this->extraData;
	if (data->pressed && !data->toggleMode)
	{
		return;
	}
	data->pressed = !data->pressed;
	this->currentSkinIndex = data->pressed ? data->onSkin : data->offSkin;
	data->timePressed = GetTimeMs();
	ActorFireOutput(this, data->pressed ? "pressed" : "unpressed", PARAM_NONE);
}

ActorDefinition buttonActorDefinition = {
	.Update = ButtonUpdate,
	.OnPlayerContactAdded = DefaultActorOnPlayerContactAdded,
	.OnPlayerContactPersisted = DefaultActorOnPlayerContactPersisted,
	.OnPlayerContactRemoved = DefaultActorOnPlayerContactRemoved,
	.RenderUi = DefaultActorRenderUi,
	.Interact = ButtonInteract,
	.Destroy = DefaultActorDestroy,
	.Init = ButtonInit,
};

void RegisterButton()
{
	RegisterDefaultActorInputs(&buttonActorDefinition);
	RegisterActor(BUTTON_ACTOR_NAME, &buttonActorDefinition);
}
