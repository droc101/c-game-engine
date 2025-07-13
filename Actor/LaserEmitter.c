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

bool LaserEmitterSignalHandler(Actor *this, const Actor *sender, byte signal, const Param *param)
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
		this->currentSkinIndex = (int)data->height + 1;
		return true;
	}
	return false;
}

void CreateLaserEmitterCollider(Actor *this, const b2WorldId worldId)
{
	b2BodyDef bodyDef = b2DefaultBodyDef();
	bodyDef.type = b2_staticBody;
	bodyDef.position = this->position;
	bodyDef.linearDamping = 10;
	bodyDef.fixedRotation = true;
	this->bodyId = b2CreateBody(worldId, &bodyDef);

	const b2Polygon sensorShape = b2MakeOffsetBox(0.1f, 0.1f, (Vector2){0, 0}, 0);
	b2ShapeDef shapeDef = b2DefaultShapeDef();
	shapeDef.filter.categoryBits = COLLISION_GROUP_DEFAULT | COLLISION_GROUP_ACTOR;
	b2CreatePolygonShape(this->bodyId, &shapeDef, &sensorShape);
}

void LaserEmitterInit(Actor *this, b2WorldId worldId, const KvList *params)
{
	this->SignalHandler = LaserEmitterSignalHandler;
	this->extraData = calloc(1, sizeof(LaserEmitterData));
	LaserEmitterData *data = this->extraData;
	data->height = (LaserHeight)KvGetByte(params, "height", LASER_HEIGHT_MIDDLE);
	data->hasTicked = false;

	this->actorModel = LoadModel(MODEL("model_laseremitter"));
	this->currentSkinIndex = (int)data->height + 1;

	CreateLaserEmitterCollider(this, worldId);

	data->startEnabled = KvGetBool(params, "startEnabled", true);
}

void LaserEmitterUpdate(Actor *this, double)
{
	LaserEmitterData *data = this->extraData;
	if (!data->hasTicked)
	{
		KvList laserParams;
		KvListCreate(&laserParams);
		KvSetByte(&laserParams, "height", data->height);
		KvSetBool(&laserParams, "startEnabled", data->startEnabled);
		data->laserActor = CreateActor(this->position,
									   this->rotation,
									   LASER_ACTOR,
									   &laserParams,
									   GetState()->level->worldId);
		AddActor(data->laserActor);
		data->hasTicked = true;
	}
}

void LaserEmitterDestroy(Actor *this)
{
	b2DestroyBody(this->bodyId);
	free(this->extraData);
}
