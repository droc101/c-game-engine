//
// Created by droc101 on 6/1/25.
//

#include "LaserEmitter.h"

#include "../Helpers/Core/AssetReader.h"
#include "../Helpers/Core/KVList.h"
#include "../Structs/Actor.h"
#include "../Structs/GlobalState.h"
#include "../Structs/Level.h"
#include "Laser.h"

#define LASER_EMITTER_INPUT_DISABLE 2
#define LASER_EMITTER_INPUT_ENABLE 1

static const Vector3 halfExtent = {0.2f, 0.48f, 0.05f};

typedef enum LaserEmitterSkin
{
	EMITTER_SKIN_OFF,
	EMITTER_SKIN_FLOOR,
	EMITTER_SKIN_MIDDLE,
	EMITTER_SKIN_CEILING,
	EMITTER_SKIN_ALL
} LaserEmitterSkin;

typedef struct LaserEmitterData
{
	LaserHeight height;
	Actor *laserActor;
	bool startEnabled;
	bool hasTicked;
} LaserEmitterData;

bool LaserEmitterSignalHandler(Actor *this, const Actor *sender, const byte signal, const Param *param)
{
	const LaserEmitterData *data = this->extraData;
	if (DefaultSignalHandler(this, sender, signal, param))
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

void CreateLaserEmitterCollider(Actor *this, const Transform *transform)
{
	JPH_Quat rotation = {};
	JPH_Quat_FromEulerAngles(&transform->rotation, &rotation);
	Vector3 forwardVector = {};
	JPH_Quat_RotateAxisZ(&rotation, &forwardVector);
	Vector3 offsetVector = {};
	JPH_Vec3_MultiplyScalar(&forwardVector, halfExtent.z, &offsetVector);
	Vector3 position = {};
	JPH_Vec3_Subtract(&transform->position, &offsetVector, &position);
	JPH_BodyCreationSettings *bodyCreationSettings = JPH_BodyCreationSettings_Create3(
			(const JPH_Shape *)JPH_BoxShape_Create(&halfExtent, JPH_DEFAULT_CONVEX_RADIUS),
			&position,
			&rotation,
			JPH_MotionType_Static,
			OBJECT_LAYER_STATIC);
	JPH_BodyCreationSettings_SetUserData(bodyCreationSettings, (uint64_t)this);
	this->bodyId = JPH_BodyInterface_CreateAndAddBody(this->bodyInterface,
													  bodyCreationSettings,
													  JPH_Activation_Activate);
	JPH_BodyCreationSettings_Destroy(bodyCreationSettings);
}

void LaserEmitterInit(Actor *this, const KvList *params, Transform *transform)
{
	this->SignalHandler = LaserEmitterSignalHandler;
	this->extraData = calloc(1, sizeof(LaserEmitterData));
	LaserEmitterData *data = this->extraData;
	data->height = (LaserHeight)KvGetByte(params, "height", LASER_HEIGHT_MIDDLE);
	data->hasTicked = false;

	this->actorModel = LoadModel(MODEL("model_laseremitter"));
	this->currentSkinIndex = data->height + 1;

	CreateLaserEmitterCollider(this, transform);

	data->startEnabled = KvGetBool(params, "startEnabled", true);

	KvList laserParams;
	KvListCreate(&laserParams);
	KvSetByte(&laserParams, "height", data->height);
	KvSetBool(&laserParams, "startEnabled", data->startEnabled);
	data->laserActor = CreateActor(transform,
								   ACTOR_TYPE_LASER,
								   &laserParams,
								   JPH_PhysicsSystem_GetBodyInterface(GetState()->level->physicsSystem));
	AddActor(data->laserActor);
	data->hasTicked = true;
}

void LaserEmitterUpdate(Actor *this, const double /*delta*/) {}
