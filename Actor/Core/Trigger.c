//
// Created by droc101 on 4/13/25.
//

#include "Trigger.h"
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
#include <stdint.h>
#include <stdlib.h>
#include "../../Helpers/Core/Error.h"
#include "../../Helpers/Core/KVList.h"
#include "../../Helpers/Core/Physics/Physics.h"
#include "../../Structs/Actor.h"
#include "../../Structs/Level.h"

enum TriggerInput
{
	TRIGGER_INPUT_FORCE_TRIGGER = 1,
	TRIGGER_INPUT_ENABLE = 2,
	TRIGGER_INPUT_DISABLE = 3,
};

enum TriggerOutput
{
	TRIGGER_OUTPUT_TRIGGERED = 2,
	TRIGGER_OUTPUT_ENTERED = 3,
	TRIGGER_OUTPUT_EXITED = 4,
};

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
	JPH_BodyCreationSettings *bodyCreationSettings = JPH_BodyCreationSettings_Create2_GAME(
			(const JPH_Shape *)JPH_BoxShape_Create((Vector3[]){{data->width / 2, 0.5f, data->depth / 2}},
												   JPH_DefaultConvexRadius),
			transform,
			JPH_MotionType_Static,
			OBJECT_LAYER_SENSOR,
			this);
	JPH_BodyCreationSettings_SetIsSensor(bodyCreationSettings, true);
	this->bodyId = JPH_BodyInterface_CreateAndAddBody(this->bodyInterface,
													  bodyCreationSettings,
													  JPH_Activation_Activate);
	JPH_BodyCreationSettings_Destroy(bodyCreationSettings);
}

static bool TriggerSignalHandler(Actor *this, const Actor *sender, const uint8_t signal, const Param *param)
{
	TriggerData *data = (TriggerData *)this->extraData;
	if (DefaultSignalHandler(this, sender, signal, param))
	{
		return true;
	}
	if (signal == TRIGGER_INPUT_FORCE_TRIGGER)
	{
		ActorFireOutput(this, TRIGGER_OUTPUT_TRIGGERED, PARAM_NONE);
		return true;
	}
	if (signal == TRIGGER_INPUT_ENABLE && !data->enabled)
	{
		data->enabled = true;
		return true;
	}
	if (signal == TRIGGER_INPUT_DISABLE && data->enabled)
	{
		data->enabled = false;
		return true;
	}
	return false;
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

void TriggerInit(Actor *this, const KvList *params, Transform *transform)
{
	this->SignalHandler = TriggerSignalHandler;
	this->OnPlayerContactAdded = TriggerOnPlayerContactAdded;
	this->OnPlayerContactPersisted = TriggerOnPlayerContactPersisted;
	this->OnPlayerContactRemoved = TriggerOnPlayerContactRemoved;

	this->extraData = malloc(sizeof(TriggerData));
	CheckAlloc(this->extraData);
	TriggerData *data = this->extraData;
	data->width = KvGetFloat(params, "width", 1.0f);
	data->depth = KvGetFloat(params, "depth", 1.0f);
	data->oneShot = KvGetBool(params, "oneShot", true);
	data->enabled = KvGetBool(params, "startEnabled", true);

	CreateTriggerSensor(this, transform);
}
