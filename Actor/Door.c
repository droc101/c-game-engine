//
// Created by droc101 on 11/7/2024.
//

#include "Door.h"
#include "../Helpers/Collision.h"
#include "../Helpers/Core/AssetReader.h"
#include "../Helpers/Core/Error.h"
#include "../Helpers/Core/KVList.h"
#include "../Helpers/Core/Logging.h"
#include "../Structs/GlobalState.h"
#include "../Structs/Vector2.h"
#include "../Structs/Wall.h"

#define DOOR_INPUT_OPEN 1
#define DOOR_INPUT_CLOSE 2

#define DOOR_OUTPUT_CLOSING 2
#define DOOR_OUTPUT_OPENING 3
#define DOOR_OUTPUT_FULLY_CLOSED 4
#define DOOR_OUTPUT_FULLY_OPEN 5

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
	JPH_Vec3_MultiplyScalar(&forwardVector, -1, &movementVector);
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
			JPH_BodyInterface_SetLinearVelocity(this->bodyInterface, this->bodyId, &JPH_Vec3_Zero);
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
			JPH_BodyInterface_SetLinearVelocity(this->bodyInterface, this->bodyId, &JPH_Vec3_Zero);
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
	JPH_Quat rotation = {};
	JPH_Quat_FromEulerAngles(&transform->rotation, &rotation);
	const Vector3 points[4] = {
		{
			0.0f,
			-0.5f,
			0.0f,
		},
		{
			0.0f,
			-0.5f,
			-1.0f,
		},
		{
			0.0f,
			0.5f,
			0.0f,
		},
		{
			0.0f,
			0.5f,
			-1.0f,
		},
	};
	const JPH_ConvexHullShapeSettings *shapeSettings = JPH_ConvexHullShapeSettings_Create(points,
																						  4,
																						  JPH_DEFAULT_CONVEX_RADIUS);
	const JPH_Shape *shape = (const JPH_Shape *)JPH_ConvexHullShapeSettings_CreateShape(shapeSettings);
	JPH_BodyCreationSettings *bodyCreationSettings = JPH_BodyCreationSettings_Create3(shape,
																					  &transform->position,
																					  &rotation,
																					  JPH_MotionType_Kinematic,
																					  OBJECT_LAYER_STATIC);
	const JPH_MassProperties massProperties = {
		.mass = 1.0f,
	};
	JPH_BodyCreationSettings_SetMassPropertiesOverride(bodyCreationSettings, &massProperties);
	JPH_BodyCreationSettings_SetOverrideMassProperties(bodyCreationSettings,
													   JPH_OverrideMassProperties_CalculateInertia);
	JPH_BodyCreationSettings_SetUserData(bodyCreationSettings, (uint64_t)this);
	this->bodyId = JPH_BodyInterface_CreateAndAddBody(this->bodyInterface,
													  bodyCreationSettings,
													  JPH_Activation_Activate);
	JPH_BodyCreationSettings_Destroy(bodyCreationSettings);

	DoorData *data = this->extraData;
	Vector3 forwardVector = {};
	JPH_Quat_RotateAxisZ(&rotation, &forwardVector);
	JPH_Vec3_Add(&transform->position, &forwardVector, &data->openPosition);
	data->closedPosition = transform->position;
}

static inline void CreateDoorSensor(Actor *this, const Transform *transform)
{
	DoorData *data = this->extraData;

	JPH_Quat rotation = {};
	JPH_Quat_FromEulerAngles(&transform->rotation, &rotation);
	Vector3 forwardVector = {};
	JPH_Quat_RotateAxisZ(&rotation, &forwardVector);
	Vector3 offsetVector = {};
	JPH_Vec3_MultiplyScalar(&forwardVector, 0.5f, &offsetVector);
	Vector3 position = {};
	JPH_Vec3_Subtract(&transform->position, &offsetVector, &position);
	const JPH_Shape *shape = (const JPH_Shape *)JPH_BoxShape_Create((Vector3[]){{0.5f, 0.5f, 0.5f}},
																	JPH_DEFAULT_CONVEX_RADIUS);
	JPH_BodyCreationSettings *bodyCreationSettings = JPH_BodyCreationSettings_Create3(shape,
																					  &position,
																					  &rotation,
																					  JPH_MotionType_Static,
																					  OBJECT_LAYER_SENSOR);
	JPH_BodyCreationSettings_SetUserData(bodyCreationSettings, (uint64_t)this);
	JPH_BodyCreationSettings_SetIsSensor(bodyCreationSettings, true);
	data->sensorBodyId = JPH_BodyInterface_CreateAndAddBody(this->bodyInterface,
															bodyCreationSettings,
															JPH_Activation_Activate);
	JPH_BodyCreationSettings_Destroy(bodyCreationSettings);
}

static bool DoorSignalHandler(Actor *this, const Actor *sender, const byte signal, const Param *param)
{
	if (DefaultSignalHandler(this, sender, signal, param))
	{
		return true;
	}
	// DoorData *data = this->extraData;
	// if (signal == DOOR_INPUT_OPEN)
	// {
	// 	if (data->state != DOOR_CLOSED)
	// 	{
	// 		if (data->state == DOOR_CLOSING)
	// 		{
	// 			b2Body_SetLinearVelocity(this->bodyId,
	// 									 Vector2Normalize(Vector2Scale(Vector2FromAngle(this->rotation), -1)));
	// 			data->state = DOOR_OPENING; // Set manually in order to not reset data->animationTime
	// 			data->animationTime = 1 - data->animationTime;
	// 		}
	// 		return true;
	// 	}
	// 	b2Body_SetLinearVelocity(this->bodyId, Vector2Normalize(Vector2Scale(Vector2FromAngle(this->rotation), -1)));
	// 	DoorSetState(this, DOOR_OPENING);
	// 	return true;
	// }
	// if (signal == DOOR_INPUT_CLOSE)
	// {
	// 	if (data->state != DOOR_OPEN)
	// 	{
	// 		if (data->state == DOOR_OPENING)
	// 		{
	// 			b2Body_SetLinearVelocity(this->bodyId, Vector2Normalize(Vector2FromAngle(this->rotation)));
	// 			data->state = DOOR_CLOSING; // Set manually in order to not reset data->animationTime
	// 			data->animationTime = 1 - data->animationTime;
	// 		}
	// 		return true;
	// 	}
	// 	b2Body_SetLinearVelocity(this->bodyId, Vector2Normalize(Vector2FromAngle(this->rotation)));
	// 	DoorSetState(this, DOOR_CLOSING);
	// 	return true;
	// }
	return false;
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
	DoorData *data = this->extraData;
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

void DoorInit(Actor *this, const KvList *params, Transform *transform)
{
	this->extraData = calloc(1, sizeof(DoorData));
	CheckAlloc(this->extraData);
	DoorData *data = this->extraData;

	CreateDoorCollider(this, transform);
	if (KvGetBool(params, "preventPlayerOpen", false))
	{
		data->sensorBodyId = JPH_BodyId_InvalidBodyID;
	} else
	{
		CreateDoorSensor(this, transform);
	}

	this->actorWall = malloc(sizeof(ActorWall));
	this->actorWall->a = v2(0, -0.5f);
	this->actorWall->b = v2(0, 0.5f);
	strncpy(this->actorWall->tex, TEXTURE("actor_door"), 80);
	this->actorWall->uvScale = 1.0f;
	this->actorWall->uvOffset = 0.0f;
	this->actorWall->height = 1.0f;
	ActorWallBake(this);

	this->SignalHandler = DoorSignalHandler;
	this->OnPlayerContactAdded = DoorOnPlayerContactAdded;
	this->OnPlayerContactPersisted = DoorOnPlayerContactPersisted;
	this->OnPlayerContactRemoved = DoorOnPlayerContactRemoved;

	data->stayOpen = KvGetBool(params, "stayOpen", false);
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void DoorUpdate(Actor *this, const double delta)
{
	// this->position = b2Body_GetPosition(this->bodyId);
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
void DoorDestroy(Actor *this)
{
	// b2DestroyBody(this->bodyId);
	// b2ShapeId *sensorShapeId = &((DoorData *)this->extraData)->sensorId;
	// b2DestroyBody(b2Shape_GetBody(*sensorShapeId));
	// *sensorShapeId = b2_nullShapeId;
}
