//
// Created by droc101 on 6/1/25.
//

#include "actor/LaserEmitter.h"
#include <joltc/enums.h>
#include <joltc/joltc.h>
#include <joltc/Math/Quat.h>
#include <joltc/Math/Transform.h>
#include <joltc/Math/Vector3.h>
#include <joltc/Physics/Body/BodyCreationSettings.h>
#include <joltc/Physics/Body/BodyInterface.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <engine/assets/ModelLoader.h>
#include <engine/assets/AssetReader.h>
#include <engine/subsystem/Error.h>
#include <engine/structs/KVList.h>
#include <engine/physics/Physics.h>
#include <engine/structs/Actor.h>
#include <engine/structs/ActorDefinition.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/Level.h>
#include <engine/structs/Param.h>
#include "actor/Laser.h"

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
	bool startOn;
	bool hasTicked;
	Transform transform;
} LaserEmitterData;

static inline void CreateLaserEmitterCollider(Actor *this, const Transform *transform)
{
	JPH_BodyCreationSettings
			*bodyCreationSettings = JPH_BodyCreationSettings_Create2_GAME(this->actorModel->collisionModelShape,
																		  transform,
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
		KvListCreate(laserParams);
		KvSetByte(laserParams, "height", data->height);
		KvSetBool(laserParams, "startEnabled", data->startOn);
		data->laserActor = CreateActor(&data->transform,
									   LASER_ACTOR_NAME,
									   laserParams,
									   JPH_PhysicsSystem_GetBodyInterface(GetState()->level->physicsSystem));
		AddActor(data->laserActor);
		data->hasTicked = true;
	}
}

static void LaserEmitterTurnOnHandler(Actor *this, const Actor * /*sender*/, const Param * /*param*/)
{
	const LaserEmitterData *data = this->extraData;
	ActorTriggerInput(this, data->laserActor, LASER_INPUT_TURN_ON, NULL);
	this->currentSkinIndex = data->height + 1;
}

static void LaserEmitterTurnOffHandler(Actor *this, const Actor * /*sender*/, const Param * /*param*/)
{
	const LaserEmitterData *data = this->extraData;
	ActorTriggerInput(this, data->laserActor, LASER_INPUT_TURN_OFF, NULL);
	this->currentSkinIndex = EMITTER_SKIN_OFF;
}

void LaserEmitterInit(Actor *this, const KvList params, Transform *transform)
{
	// TODO: uncomment once laser emitter collision has holes for where the laser comes out
	this->actorFlags = ACTOR_FLAG_CAN_BLOCK_LASERS;

	this->extraData = calloc(1, sizeof(LaserEmitterData));
	CheckAlloc(this->extraData);
	LaserEmitterData *data = this->extraData;
	data->height = (LaserHeight)KvGetByte(params, "height", LASER_HEIGHT_MIDDLE);
	data->hasTicked = false;

	this->actorModel = LoadModel(MODEL("laseremitter"));
	this->currentSkinIndex = data->height + 1;

	data->transform = *transform;
	Vector3 forwardVector = {};
	JPH_Quat_RotateAxisZ(&transform->rotation, &forwardVector);
	Vector3 offsetVector = {};
	Vector3_MultiplyScalar(&forwardVector, this->actorModel->boundingBoxExtents.z, &offsetVector);
	Vector3_Subtract(&transform->position, &offsetVector, &data->transform.position);

	CreateLaserEmitterCollider(this, transform);

	data->startOn = KvGetBool(params, "startOn", true);
}

static ActorDefinition definition = {.actorType = ACTOR_TYPE_LASER_EMITTER,
									 .Update = LaserEmitterUpdate,
									 .OnPlayerContactAdded = DefaultActorOnPlayerContactAdded,
									 .OnPlayerContactPersisted = DefaultActorOnPlayerContactPersisted,
									 .OnPlayerContactRemoved = DefaultActorOnPlayerContactRemoved,
									 .RenderUi = DefaultActorRenderUi,
									 .Destroy = DefaultActorDestroy,
									 .Init = LaserEmitterInit};

void RegisterLaserEmitter()
{
	RegisterDefaultActorInputs(&definition);
	RegisterActorInput(&definition, LASER_EMITTER_INPUT_TURN_ON, LaserEmitterTurnOnHandler);
	RegisterActorInput(&definition, LASER_EMITTER_INPUT_TURN_OFF, LaserEmitterTurnOffHandler);
	RegisterActor(LASER_EMITTER_ACTOR_NAME, &definition);
}
