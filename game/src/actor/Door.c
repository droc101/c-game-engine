//
// Created by droc101 on 11/7/2024.
//

#include "actor/Door.h"
#include <engine/assets/AssetReader.h>
#include <engine/physics/Physics.h>
#include <engine/structs/Actor.h>
#include <engine/structs/ActorDefinition.h>
#include <engine/structs/ActorWall.h>
#include <engine/structs/Color.h>
#include <engine/structs/KVList.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Logging.h>
#include <joltc/constants.h>
#include <joltc/enums.h>
#include <joltc/joltc.h>
#include <joltc/Math/Quat.h>
#include <joltc/Math/Transform.h>
#include <joltc/Math/Vector3.h>
#include <joltc/Physics/Body/BodyCreationSettings.h>
#include <joltc/Physics/Body/BodyID.h>
#include <joltc/Physics/Body/BodyInterface.h>
#include <joltc/Physics/Body/MassProperties.h>
#include <joltc/Physics/Collision/Shape/Shape.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

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
	JPH_BodyID sensorBodyId;
	Vector3 closedPosition;
	Vector3 openPosition;
	float width;
	float stayOpenTime;
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
			ActorFireOutput(this, DOOR_OUTPUT_FULLY_OPENED, PARAM_NONE);
			break;
		case DOOR_CLOSING:
			DoorSetCloseVector(this);
			ActorFireOutput(this, DOOR_OUTPUT_CLOSING, PARAM_NONE);
			break;
	}
}

static inline void CreateDoorCollider(Actor *this, const Transform *transform)
{
	JPH_Shape *shape = ActorWallCreateCollider(this->wall);
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

	JPH_Shape *shape = (JPH_Shape *)
			JPH_BoxShape_Create((Vector3[]){{0.5f, this->wall->height / 2.0f, data->width / 2.0f}},
								JPH_DefaultConvexRadius);
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
	Vector3_MultiplyScalar(&forwardVector, data->width / 2.0f, &offsetVector);
	Vector3_Subtract(&transform->position, &offsetVector, &data->closedPosition);
	Vector3_Add(&transform->position, &offsetVector, &data->openPosition);

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

static void DoorUpdate(Actor *this, const double delta)
{
	DoorData *data = this->extraData;
	switch (data->state)
	{
		case DOOR_OPENING:
			if (data->animationTime >= data->width)
			{
				DoorSetState(this, DOOR_OPEN, 0);
			}
			break;
		case DOOR_OPEN:
			if (data->animationTime >= data->stayOpenTime && data->shouldClose)
			{
				DoorSetState(this, DOOR_CLOSING, 0);
				data->shouldClose = false;
			}
			break;
		case DOOR_CLOSING:
			if (data->animationTime >= data->width)
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

static void DoorDestroy(Actor *this)
{
	const DoorData *data = this->extraData;
	if (data->sensorBodyId != JPH_BodyId_InvalidBodyID && this->bodyInterface != NULL)
	{
		JPH_BodyInterface_RemoveAndDestroyBody(this->bodyInterface, data->sensorBodyId);
	}
}

static void DoorOpenHandler(Actor *this, const Actor * /*sender*/, const Param * /*param*/)
{
	const DoorData *data = this->extraData;
	switch (data->state)
	{
		case DOOR_CLOSED:
			DoorSetState(this, DOOR_OPENING, 0);
			return;
		case DOOR_CLOSING:
			DoorSetState(this, DOOR_OPENING, data->width - data->animationTime);
		default:
	}
}

static void DoorCloseHandler(Actor *this, const Actor * /*sender*/, const Param * /*param*/)
{
	const DoorData *data = this->extraData;
	switch (data->state)
	{
		case DOOR_OPEN:
			DoorSetState(this, DOOR_CLOSING, 0);
			return;
		case DOOR_OPENING:
			DoorSetState(this, DOOR_CLOSING, data->width - data->animationTime);
		default:
	}
}

static void DoorOnPlayerContactAdded(Actor *this, const JPH_BodyID bodyId)
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
			DoorSetState(this, DOOR_OPENING, data->width - data->animationTime);
			break;
		case DOOR_OPEN:
		case DOOR_OPENING:
			break;
		default:
			LogWarning("Invalid door state: %d", data->state);
			break;
	}
}

static void DoorOnPlayerContactPersisted(Actor *this, const JPH_BodyID bodyId)
{
	const DoorData *data = this->extraData;
	if (bodyId != data->sensorBodyId)
	{
		return;
	}
	switch (data->state)
	{
		case DOOR_OPENING:
			if (data->animationTime >= data->width)
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

static void DoorOnPlayerContactRemoved(Actor *this, const JPH_BodyID bodyId)
{
	DoorData *data = this->extraData;
	if (bodyId != data->sensorBodyId)
	{
		return;
	}
	switch (data->state)
	{
		case DOOR_OPEN:
			if (!data->stayOpen && data->animationTime >= data->width)
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

void DoorInit(Actor *this, const KvList params, Transform *transform)
{
	this->flags = ACTOR_FLAG_CAN_PUSH_PLAYER | ACTOR_FLAG_CAN_BLOCK_LASERS;

	const Vector2 size = KvGetVec2(params, "size", v2s(1.0f));

	this->extraData = calloc(1, sizeof(DoorData));
	CheckAlloc(this->extraData);
	DoorData *data = this->extraData;
	data->stayOpen = KvGetBool(params, "stayOpen", false);
	data->width = size.x;
	data->stayOpenTime = KvGetFloat(params, "delay_until_close", 1.0f);

	this->wall = malloc(sizeof(ActorWall));
	CheckAlloc(this->wall);
	const float width = data->width;
	this->wall->a = v2(0, -width / 2.0f);
	this->wall->b = v2(0, width / 2.0f);
	this->wall->tex = strdup(KvGetString(params, "texture", TEXTURE("actor/door")));
	this->wall->uvScale = KvGetVec2(params, "uv_scale", v2s(1.0f));
	this->wall->uvOffset = KvGetVec2(params, "uv_offset", v2s(0.0f));
	this->wall->height = size.y;
	this->wall->unshaded = KvGetBool(params, "unshaded", false);
	this->modColor = KvGetColor(params, "color", COLOR_WHITE);
	ActorWallBake(this);

	CreateDoorBodies(this, transform, KvGetBool(params, "preventPlayerOpen", false));
}

ActorDefinition doorActorDefinition = {
	.Update = DoorUpdate,
	.OnPlayerContactAdded = DoorOnPlayerContactAdded,
	.OnPlayerContactPersisted = DoorOnPlayerContactPersisted,
	.OnPlayerContactRemoved = DoorOnPlayerContactRemoved,
	.RenderUi = DefaultActorRenderUi,
	.Interact = DefaultActorInteract,
	.Destroy = DoorDestroy,
	.Init = DoorInit,
};

void RegisterDoor()
{
	RegisterDefaultActorInputs(&doorActorDefinition);
	RegisterActorInput(&doorActorDefinition, DOOR_INPUT_OPEN, DoorOpenHandler);
	RegisterActorInput(&doorActorDefinition, DOOR_INPUT_CLOSE, DoorCloseHandler);
	RegisterActor(DOOR_ACTOR_NAME, &doorActorDefinition);
}
