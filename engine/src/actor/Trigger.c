//
// Created by droc101 on 4/13/25.
//

#include <engine/actor/Trigger.h>
#include <engine/physics/Physics.h>
#include <engine/structs/Actor.h>
#include <engine/structs/ActorDefinition.h>
#include <engine/structs/KVList.h>
#include <engine/structs/Map.h>
#include <engine/structs/Param.h>
#include <engine/subsystem/Error.h>
#include <joltc/constants.h>
#include <joltc/enums.h>
#include <joltc/joltc.h>
#include <joltc/Math/Transform.h>
#include <joltc/Math/Vector3.h>
#include <joltc/Physics/Body/BodyCreationSettings.h>
#include <joltc/Physics/Body/BodyInterface.h>
#include <joltc/Physics/Collision/Shape/Shape.h>
#include <joltc/types.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct TriggerData
{
	float width;
	float depth;
	bool oneShot;
	bool enabled;
} TriggerData;

static inline void CreateTriggerSensor(Actor *this, const Transform *transform)
{
	const TriggerData *data = this->extraData;
	JPH_Shape *shape = (JPH_Shape *)JPH_BoxShape_Create((Vector3[]){{data->width / 2, 0.5f, data->depth / 2}},
														JPH_DefaultConvexRadius);
	JPH_BodyCreationSettings *bodyCreationSettings = JPH_BodyCreationSettings_Create2_GAME(shape,
																						   transform,
																						   JPH_MotionType_Static,
																						   OBJECT_LAYER_SENSOR,
																						   this);
	JPH_BodyCreationSettings_SetIsSensor(bodyCreationSettings, true);
	this->bodyId = JPH_BodyInterface_CreateAndAddBody(this->bodyInterface,
													  bodyCreationSettings,
													  JPH_Activation_Activate);
	JPH_Shape_Destroy(shape);
	JPH_BodyCreationSettings_Destroy(bodyCreationSettings);
}

static void TriggerForceTriggerHandler(Actor *this, const Actor * /*sender*/, const Param * /*param*/)
{
	ActorFireOutput(this, TRIGGER_OUTPUT_TRIGGERED, PARAM_NONE);
}

static void TriggerEnableHandler(Actor *this, const Actor * /*sender*/, const Param * /*param*/)
{
	TriggerData *data = (TriggerData *)this->extraData;
	data->enabled = true;
}

static void TriggerDisableHandler(Actor *this, const Actor * /*sender*/, const Param * /*param*/)
{
	TriggerData *data = (TriggerData *)this->extraData;
	data->enabled = false;
}

static void TriggerOnPlayerContactAdded(Actor *this, JPH_BodyId /*bodyId*/)
{
	const TriggerData *data = this->extraData;
	if (data->enabled)
	{
		ActorFireOutput(this, TRIGGER_OUTPUT_ENTERED, PARAM_NONE);
		ActorFireOutput(this, TRIGGER_OUTPUT_TRIGGERED, PARAM_NONE);
	}
}

static void TriggerOnPlayerContactPersisted(Actor *this, JPH_BodyId /*bodyId*/)
{
	const TriggerData *data = this->extraData;
	if (!data->oneShot && data->enabled)
	{
		ActorFireOutput(this, TRIGGER_OUTPUT_TRIGGERED, PARAM_NONE);
	}
}

static void TriggerOnPlayerContactRemoved(Actor *this, JPH_BodyId /*bodyId*/)
{
	const TriggerData *data = this->extraData;
	if (data->enabled)
	{
		ActorFireOutput(this, TRIGGER_OUTPUT_EXITED, PARAM_NONE);
		if (data->oneShot)
		{
			RemoveActor(this);
		}
	}
}

void TriggerInit(Actor *this, const KvList params, Transform *transform)
{
	this->extraData = malloc(sizeof(TriggerData));
	CheckAlloc(this->extraData);
	TriggerData *data = this->extraData;
	data->width = KvGetFloat(params, "width", 1.0f);
	data->depth = KvGetFloat(params, "depth", 1.0f);
	data->oneShot = KvGetBool(params, "oneShot", true);
	data->enabled = KvGetBool(params, "startEnabled", true);

	CreateTriggerSensor(this, transform);
}

static ActorDefinition definition = {
	.actorType = ACTOR_TYPE_TRIGGER,
	.Update = DefaultActorUpdate,
	.OnPlayerContactAdded = TriggerOnPlayerContactAdded,
	.OnPlayerContactPersisted = TriggerOnPlayerContactPersisted,
	.OnPlayerContactRemoved = TriggerOnPlayerContactRemoved,
	.RenderUi = DefaultActorRenderUi,
	.Destroy = DefaultActorDestroy,
	.Init = TriggerInit,
};

void RegisterTrigger()
{
	RegisterDefaultActorInputs(&definition);
	RegisterActorInput(&definition, TRIGGER_INPUT_FORCE_TRIGGER, TriggerForceTriggerHandler);
	RegisterActorInput(&definition, TRIGGER_INPUT_ENABLE, TriggerEnableHandler);
	RegisterActorInput(&definition, TRIGGER_INPUT_DISABLE, TriggerDisableHandler);
	RegisterActor(TRIGGER_ACTOR_NAME, &definition);
}
