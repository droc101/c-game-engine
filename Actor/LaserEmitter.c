//
// Created by droc101 on 6/1/25.
//

#include "LaserEmitter.h"
#include <joltc/constants.h>
#include <joltc/enums.h>
#include <joltc/joltc.h>
#include <joltc/Math/Quat.h>
#include <joltc/Math/Transform.h>
#include <joltc/Math/Vector3.h>
#include <joltc/Physics/Body/BodyCreationSettings.h>
#include <joltc/Physics/Body/BodyInterface.h>
#include <joltc/Physics/Collision/Shape/Shape.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include "../Helpers/Core/AssetLoaders/ModelLoader.h"
#include "../Helpers/Core/AssetReader.h"
#include "../Helpers/Core/Error.h"
#include "../Helpers/Core/KVList.h"
#include "../Helpers/Core/Physics/Physics.h"
#include "../Structs/Actor.h"
#include "../Structs/ActorDefinitions.h"
#include "../Structs/GlobalState.h"
#include "../Structs/Level.h"
#include "Laser.h"

enum LaserEmitterInputs
{
	LASER_EMITTER_INPUT_ENABLE = 1,
	LASER_EMITTER_INPUT_DISABLE = 2,
};

enum LaserEmitterSkin
{
	EMITTER_SKIN_OFF,
	EMITTER_SKIN_FLOOR,
	EMITTER_SKIN_MIDDLE,
	EMITTER_SKIN_CEILING,
	EMITTER_SKIN_ALL
};

typedef struct LaserEmitterData
{
	LaserHeight height;
	Actor *laserActor;
	bool startEnabled;
	bool hasTicked;
	Transform transform;
} LaserEmitterData;

static inline void CreateLaserEmitterCollider(Actor *this, const Transform *transform)
{
	JPH_BodyCreationSettings *bodyCreationSettings = CreateBoundingBoxBodyCreationSettings(transform,
																						   this->actorModel,
																						   JPH_MotionType_Static,
																						   OBJECT_LAYER_STATIC,
																						   this);
	this->bodyId = JPH_BodyInterface_CreateAndAddBody(this->bodyInterface,
													  bodyCreationSettings,
													  JPH_Activation_Activate);
	JPH_BodyCreationSettings_Destroy(bodyCreationSettings);
}

static void LaserEmitterUpdate(Actor *this, const double /*delta*/)
{
	LaserEmitterData *data = this->extraData;
	if (!data->hasTicked)
	{
		KvList laserParams;
		KvListCreate(&laserParams);
		KvSetByte(&laserParams, "height", data->height);
		KvSetBool(&laserParams, "startEnabled", data->startEnabled);
		data->laserActor = CreateActor(&data->transform,
									   "laser",
									   &laserParams,
									   JPH_PhysicsSystem_GetBodyInterface(GetState()->level->physicsSystem));
		AddActor(data->laserActor);
		data->hasTicked = true;
	}
}

static bool LaserEmitterSignalHandler(Actor *this, const Actor *sender, const uint8_t signal, const Param *param)
{
	const LaserEmitterData *data = this->extraData;
	if (DefaultActorSignalHandler(this, sender, signal, param))
	{
		return true;
	}
	if (signal == LASER_EMITTER_INPUT_DISABLE)
	{
		ActorTriggerInput(this, data->laserActor, LASER_INPUT_DISABLE, NULL);
		this->currentSkinIndex = EMITTER_SKIN_OFF;
		return true;
	}
	if (signal == LASER_EMITTER_INPUT_ENABLE)
	{
		ActorTriggerInput(this, data->laserActor, LASER_INPUT_ENABLE, NULL);
		this->currentSkinIndex = data->height + 1;
		return true;
	}
	return false;
}

static ActorDefinition definition = {
	.actorType = ACTOR_TYPE_LASER_EMITTER,
	.Update = LaserEmitterUpdate,
	.SignalHandler = LaserEmitterSignalHandler,
	.OnPlayerContactAdded = DefaultActorOnPlayerContactAdded,
	.OnPlayerContactPersisted = DefaultActorOnPlayerContactPersisted,
	.OnPlayerContactRemoved = DefaultActorOnPlayerContactRemoved,
	.RenderUi = DefaultActorRenderUi,
	.Destroy = DefaultActorDestroy,
};

void LaserEmitterInit(Actor *this, const KvList *params, Transform *transform)
{
	this->definition = &definition;
	//this->actorFlags = ACTOR_FLAG_CAN_BLOCK_LASERS; // TODO uncomment once laser emitter collision has holes for where the laser comes out

	this->extraData = calloc(1, sizeof(LaserEmitterData));
	CheckAlloc(this->extraData);
	LaserEmitterData *data = this->extraData;
	data->height = (LaserHeight)KvGetByte(params, "height", LASER_HEIGHT_MIDDLE);
	data->hasTicked = false;

	this->actorModel = LoadModel(MODEL("laseremitter"));
	this->currentSkinIndex = data->height + 1;

	JPH_Quat rotation = {};
	JPH_Quat_FromEulerAngles(&transform->rotation, &rotation);
	Vector3 forwardVector = {};
	JPH_Quat_RotateAxisZ(&rotation, &forwardVector);
	Vector3 offsetVector = {};
	Vector3_MultiplyScalar(&forwardVector, this->actorModel->boundingBoxExtents.z, &offsetVector);
	Vector3_Subtract(&transform->position, &offsetVector, &data->transform.position);
	data->transform.rotation = transform->rotation;

	CreateLaserEmitterCollider(this, &data->transform);

	data->startEnabled = KvGetBool(params, "startEnabled", true);
}
