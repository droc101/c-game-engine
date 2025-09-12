//
// Created by droc101 on 11/7/2024.
//

#include "Door.h"
#include <joltc/constants.h>
#include <joltc/enums.h>
#include <joltc/joltc.h>
#include <joltc/Math/Quat.h>
#include <joltc/Math/Transform.h>
#include <joltc/Math/Vector3.h>
#include <joltc/Physics/Body/BodyCreationSettings.h>
#include <joltc/Physics/Body/BodyInterface.h>
#include <joltc/Physics/Body/MassProperties.h>
#include <joltc/Physics/Collision/Shape/Shape.h>
#include <joltc/types.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "../config.h"
#include "../Helpers/Core/AssetReader.h"
#include "../Helpers/Core/Error.h"
#include "../Helpers/Core/KVList.h"
#include "../Helpers/Core/Logging.h"
#include "../Helpers/Core/Physics/Physics.h"
#include "../Structs/Actor.h"
#include "../Structs/ActorDefinition.h"
#include "../Structs/Vector2.h"
#include "../Structs/Wall.h"

enum DoorInput
{
	DOOR_INPUT_OPEN = 1,
	DOOR_INPUT_CLOSE = 2,
};

enum DoorOutput
{
	DOOR_OUTPUT_CLOSING = 2,
	DOOR_OUTPUT_OPENING = 3,
	DOOR_OUTPUT_FULLY_CLOSED = 4,
	DOOR_OUTPUT_FULLY_OPEN = 5,
};

typedef enum
{
	DOOR_CLOSED,
	DOOR_OPENING,
	DOOR_OPEN,
	DOOR_CLOSING
} DoorState;

typedef struct DoorData
{
	DoorState state;
	bool shouldClose;
	bool stayOpen;
	double animationTime;
	JPH_BodyId sensorBodyId;
	Vector3 closedPosition;
	Vector3 openPosition;
} DoorData;

static inline void DoorSetOpenVector(const Actor *this)
{
	JPH_Quat rotation = {};
	JPH_BodyInterface_GetRotation(this->bodyInterface, this->bodyId, &rotation);
	Vector3 movementVector = {};
	JPH_Quat_RotateAxisZ(&rotation, &movementVector);
	JPH_BodyInterface_SetLinearVelocity(this->bodyInterface, this->bodyId, &movementVector);
}

static inline void DoorSetCloseVector(const Actor *this)
{
	JPH_Quat rotation = {};
	JPH_BodyInterface_GetRotation(this->bodyInterface, this->bodyId, &rotation);
	Vector3 forwardVector = {};
	JPH_Quat_RotateAxisZ(&rotation, &forwardVector);
	Vector3 movementVector = {};
	Vector3_MultiplyScalar(&forwardVector, -1, &movementVector);
	JPH_BodyInterface_SetLinearVelocity(this->bodyInterface, this->bodyId, &movementVector);
}

static inline void DoorSetState(const Actor *this, const DoorState state, const double animationTime)
{
	DoorData *data = this->extraData;
	data->state = state;
	data->animationTime = animationTime;
	switch (state)
	{
		case DOOR_CLOSED:
			JPH_BodyInterface_SetLinearVelocity(this->bodyInterface, this->bodyId, &Vector3_Zero);
			JPH_BodyInterface_SetPosition(this->bodyInterface,
										  this->bodyId,
										  &data->closedPosition,
										  JPH_Activation_DontActivate);
			ActorFireOutput(this, DOOR_OUTPUT_FULLY_CLOSED, PARAM_NONE);
			break;
		case DOOR_OPENING:
			DoorSetOpenVector(this);
			ActorFireOutput(this, DOOR_OUTPUT_OPENING, PARAM_NONE);
			break;
		case DOOR_OPEN:
			JPH_BodyInterface_SetLinearVelocity(this->bodyInterface, this->bodyId, &Vector3_Zero);
			JPH_BodyInterface_SetPosition(this->bodyInterface,
										  this->bodyId,
										  &data->openPosition,
										  JPH_Activation_DontActivate);
			ActorFireOutput(this, DOOR_OUTPUT_FULLY_OPEN, PARAM_NONE);
			break;
		case DOOR_CLOSING:
			DoorSetCloseVector(this);
			ActorFireOutput(this, DOOR_OUTPUT_CLOSING, PARAM_NONE);
			break;
	}
}

static inline void CreateDoorCollider(Actor *this, const Transform *transform)
{
	JPH_Shape *shape = ActorWallCreateCollider();
	JPH_BodyCreationSettings *bodyCreationSettings = JPH_BodyCreationSettings_Create2_GAME(shape,
																						   transform,
																						   JPH_MotionType_Kinematic,
																						   OBJECT_LAYER_STATIC,
																						   this);
	const JPH_MassProperties massProperties = {
		.mass = 1.0f,
	};
	JPH_BodyCreationSettings_SetMassPropertiesOverride(bodyCreationSettings, &massProperties);
	JPH_BodyCreationSettings_SetOverrideMassProperties(bodyCreationSettings,
													   JPH_OverrideMassProperties_CalculateInertia);
	this->bodyId = JPH_BodyInterface_CreateAndAddBody(this->bodyInterface,
													  bodyCreationSettings,
													  JPH_Activation_Activate);
	JPH_Shape_Destroy(shape);
	JPH_BodyCreationSettings_Destroy(bodyCreationSettings);
}

static inline void CreateDoorSensor(Actor *this, const Transform *transform)
{
	DoorData *data = this->extraData;

	JPH_Shape *shape = (JPH_Shape *)JPH_BoxShape_Create((Vector3[]){{0.5f, 0.5f, 0.5f}}, JPH_DefaultConvexRadius);
	JPH_BodyCreationSettings *bodyCreationSettings = JPH_BodyCreationSettings_Create2_GAME(shape,
																						   transform,
																						   JPH_MotionType_Static,
																						   OBJECT_LAYER_SENSOR,
																						   this);
	JPH_BodyCreationSettings_SetIsSensor(bodyCreationSettings, true);
	data->sensorBodyId = JPH_BodyInterface_CreateAndAddBody(this->bodyInterface,
															bodyCreationSettings,
															JPH_Activation_Activate);
	JPH_Shape_Destroy(shape);
	JPH_BodyCreationSettings_Destroy(bodyCreationSettings);
}

static inline void CreateDoorBodies(Actor *this, const Transform *transform, const bool preventPlayerOpen)
{
	DoorData *data = this->extraData;
	Vector3 forwardVector = {};
	JPH_Quat_RotateAxisZ(&transform->rotation, &forwardVector);
	Vector3 offsetVector = {};
	Vector3_MultiplyScalar(&forwardVector, 0.5f, &offsetVector);
	Vector3_Subtract(&transform->position, &offsetVector, &data->closedPosition);
	Vector3_Add(&data->closedPosition, &forwardVector, &data->openPosition);

	Transform calculatedTransform = *transform;
	calculatedTransform.position = data->closedPosition;

	CreateDoorCollider(this, &calculatedTransform);
	if (preventPlayerOpen)
	{
		data->sensorBodyId = JPH_BodyId_InvalidBodyID;
	} else
	{
		CreateDoorSensor(this, &calculatedTransform);
	}
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
static void DoorUpdate(Actor *this, const double delta)
{
	DoorData *data = this->extraData;
	switch (data->state)
	{
		case DOOR_OPENING:
			if (data->animationTime >= 1)
			{
				DoorSetState(this, DOOR_OPEN, 0);
			}
			break;
		case DOOR_OPEN:
			if (data->animationTime >= 1 && data->shouldClose)
			{
				DoorSetState(this, DOOR_CLOSING, 0);
				data->shouldClose = false;
			}
			break;
		case DOOR_CLOSING:
			if (data->animationTime >= 1)
			{
				DoorSetState(this, DOOR_CLOSED, 0);
				data->shouldClose = false;
			}
			break;
		default:
			break;
	}
	data->animationTime += delta / PHYSICS_TARGET_TPS;
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
static void DoorDestroy(Actor *this)
{
	const DoorData *data = this->extraData;
	if (data->sensorBodyId != JPH_BodyId_InvalidBodyID && this->bodyInterface != NULL)
	{
		JPH_BodyInterface_RemoveAndDestroyBody(this->bodyInterface, data->sensorBodyId);
	}
}

static bool DoorSignalHandler(Actor *this, const Actor *sender, const uint8_t signal, const Param *param)
{
	if (DefaultActorSignalHandler(this, sender, signal, param))
	{
		return true;
	}
	const DoorData *data = this->extraData;
	switch (signal)
	{
		case DOOR_INPUT_OPEN:
			switch (data->state)
			{
				case DOOR_CLOSED:
					DoorSetState(this, DOOR_OPENING, 0);
					return true;
				case DOOR_CLOSING:
					DoorSetState(this, DOOR_OPENING, 1 - data->animationTime);
					return true;
				default:
					return true;
			}
		case DOOR_INPUT_CLOSE:
			switch (data->state)
			{
				case DOOR_OPEN:
					DoorSetState(this, DOOR_CLOSING, 0);
					return true;
				case DOOR_OPENING:
					DoorSetState(this, DOOR_CLOSING, 1 - data->animationTime);
					return true;
				default:
					return true;
			}
		default:
			return false;
	}
}

static void DoorOnPlayerContactAdded(Actor *this, const JPH_BodyId bodyId)
{
	DoorData *data = this->extraData;
	if (bodyId != data->sensorBodyId)
	{
		return;
	}
	data->shouldClose = false;
	switch (data->state)
	{
		case DOOR_CLOSED:
			DoorSetState(this, DOOR_OPENING, 0);
			break;
		case DOOR_CLOSING:
			DoorSetState(this, DOOR_OPENING, 1 - data->animationTime);
			break;
		case DOOR_OPEN:
		case DOOR_OPENING:
			break;
		default:
			LogWarning("Invalid door state: %d", data->state);
			break;
	}
}

static void DoorOnPlayerContactPersisted(Actor *this, const JPH_BodyId bodyId)
{
	const DoorData *data = this->extraData;
	if (bodyId != data->sensorBodyId)
	{
		return;
	}
	switch (data->state)
	{
		case DOOR_OPENING:
			if (data->animationTime >= 1)
			{
				DoorSetState(this, DOOR_OPEN, 0);
			}
			break;
		case DOOR_OPEN:
			break;
		default:
			LogWarning("Invalid door state: %d", data->state);
			break;
	}
}

static void DoorOnPlayerContactRemoved(Actor *this, const JPH_BodyId bodyId)
{
	DoorData *data = this->extraData;
	if (bodyId != data->sensorBodyId)
	{
		return;
	}
	switch (data->state)
	{
		case DOOR_OPEN:
			if (!data->stayOpen && data->animationTime >= 1)
			{
				DoorSetState(this, DOOR_CLOSING, 0);
			} else
			{
				data->shouldClose = !data->stayOpen;
			}
			break;
		case DOOR_OPENING:
			data->shouldClose = !data->stayOpen;
			break;
		case DOOR_CLOSED:
		case DOOR_CLOSING:
			break;
		default:
			LogWarning("Invalid door state: %d", data->state);
			break;
	}
}

static ActorDefinition definition = {
	.actorType = ACTOR_TYPE_DOOR,
	.Update = DoorUpdate,
	.SignalHandler = DoorSignalHandler,
	.OnPlayerContactAdded = DoorOnPlayerContactAdded,
	.OnPlayerContactPersisted = DoorOnPlayerContactPersisted,
	.OnPlayerContactRemoved = DoorOnPlayerContactRemoved,
	.RenderUi = DefaultActorRenderUi,
	.Destroy = DoorDestroy,
};

void DoorInit(Actor *this, const KvList params, Transform *transform)
{
	this->definition = &definition;

	this->actorFlags = ACTOR_FLAG_CAN_PUSH_PLAYER | ACTOR_FLAG_CAN_BLOCK_LASERS;

	this->extraData = calloc(1, sizeof(DoorData));
	CheckAlloc(this->extraData);
	DoorData *data = this->extraData;
	data->stayOpen = KvGetBool(params, "stayOpen", false);

	CreateDoorBodies(this, transform, KvGetBool(params, "preventPlayerOpen", false));

	this->actorWall = malloc(sizeof(ActorWall));
	this->actorWall->a = v2(0, -0.5f);
	this->actorWall->b = v2(0, 0.5f);
	strncpy(this->actorWall->tex, TEXTURE("actor/door"), 80);
	this->actorWall->uvScale = 1.0f;
	this->actorWall->uvOffset = 0.0f;
	this->actorWall->height = 1.0f;
	ActorWallBake(this);
}
