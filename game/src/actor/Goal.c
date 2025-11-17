//
// Created by droc101 on 7/12/2024.
//

#include "actor/Goal.h"
#include <cglm/types.h>
#include <engine/assets/AssetReader.h>
#include <engine/physics/Physics.h>
#include <engine/structs/Actor.h>
#include <engine/structs/ActorDefinition.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/KVList.h>
#include <engine/structs/Map.h>
#include <engine/structs/Param.h>
#include <engine/structs/Vector2.h>
#include <engine/structs/Wall.h>
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
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef struct GoalData
{
	bool enabled;
} GoalData;

static inline void CreateGoalSensor(Actor *this, const Transform *transform)
{
	JPH_Shape *shape = (JPH_Shape *)JPH_BoxShape_Create((Vector3[]){{0.5f, 0.5f, 0.5f}}, JPH_DefaultConvexRadius);
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

static void GoalUpdate(Actor *this, double /*delta*/)
{
	Vector3 position = {};
	JPH_BodyInterface_GetPosition(this->bodyInterface, this->bodyId, &position);
	const float rotation = atan2f(GetState()->level->player.transform.position.z - position.z,
								  GetState()->level->player.transform.position.x - position.x) +
						   GLM_PI_2f;
	this->actorWall->a = v2(0.5f * cosf(rotation), 0.5f * sinf(rotation));
	this->actorWall->b = v2(-0.5f * cosf(rotation), -0.5f * sinf(rotation));
}

static void GoalEnableHandler(Actor *this, const Actor * /*sender*/, const Param * /*param*/)
{
	GoalData *data = this->extraData;
	data->enabled = true;
}

static void GoalDisableHandler(Actor *this, const Actor * /*sender*/, const Param * /*param*/)
{
	GoalData *data = this->extraData;
	data->enabled = false;
}

static void GoalOnPlayerContactAdded(Actor *this, JPH_BodyId /*bodyId*/)
{
	const GoalData *data = this->extraData;
	if (data->enabled)
	{
		GetState()->saveData->coins += 10;
		ActorFireOutput(this, GOAL_OUTPUT_COLLECTED, PARAM_NONE);
		RemoveActor(this);
	}
}

void GoalInit(Actor *this, const KvList params, Transform *transform)
{
	GoalData *data = calloc(1, sizeof(GoalData));
	CheckAlloc(data);
	this->extraData = data;
	data->enabled = KvGetBool(params, "startEnabled", true);

	this->actorWall = malloc(sizeof(ActorWall));
	CheckAlloc(this->actorWall);
	this->actorWall->a = v2(0, 0.5f);
	this->actorWall->b = v2(0, -0.5f);
	strncpy(this->actorWall->tex, data->enabled ? TEXTURE("actor/goal0") : TEXTURE("actor/goal1"), 80);
	this->actorWall->uvScale = 1.0f;
	this->actorWall->uvOffset = 0.0f;
	this->actorWall->height = 1.0f;
	ActorWallBake(this);

	const Transform adjustedTransform = {
		.position = transform->position,
		.rotation.w = 1.0f,
	};
	CreateGoalSensor(this, &adjustedTransform);
}

static ActorDefinition definition = {.actorType = ACTOR_TYPE_GOAL,
									 .Update = GoalUpdate,
									 .OnPlayerContactAdded = GoalOnPlayerContactAdded,
									 .OnPlayerContactPersisted = DefaultActorOnPlayerContactPersisted,
									 .OnPlayerContactRemoved = DefaultActorOnPlayerContactRemoved,
									 .RenderUi = DefaultActorRenderUi,
									 .Destroy = DefaultActorDestroy,
									 .Init = GoalInit};

void RegisterGoal()
{
	RegisterDefaultActorInputs(&definition);
	RegisterActorInput(&definition, GOAL_INPUT_ENABLE, GoalEnableHandler);
	RegisterActorInput(&definition, GOAL_INPUT_DISABLE, GoalDisableHandler);
	RegisterActor(GOAL_ACTOR_NAME, &definition);
}
