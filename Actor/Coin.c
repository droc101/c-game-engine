//
// Created by droc101 on 7/11/2024.
//

#include "Coin.h"
#include "../Helpers/Core/AssetReader.h"
#include "../Helpers/Core/Error.h"
#include "../Helpers/Core/KVList.h"
#include "../Structs/Actor.h"
#include "../Structs/GlobalState.h"
#include "../Structs/Level.h"
#include "../Structs/Vector2.h"

#define COIN_OUTPUT_COLLECTED 2

typedef struct CoinData
{
	bool isBlue;
	byte currentAnimationFrame;
} CoinData;

void CreateCoinSensor(Actor *this, const Transform *transform)
{
	const JPH_Shape *shape = (const JPH_Shape *)JPH_BoxShape_Create((Vector3[]){{0.25f, 0.25f, 0.25f}},
																	JPH_DEFAULT_CONVEX_RADIUS);
	JPH_BodyCreationSettings *bodyCreationSettings = JPH_BodyCreationSettings_Create3(shape,
																					  &transform->position,
																					  &JPH_Quat_Identity,
																					  JPH_MotionType_Static,
																					  OBJECT_LAYER_SENSOR);
	JPH_BodyCreationSettings_SetUserData(bodyCreationSettings, (uint64_t)this);
	JPH_BodyCreationSettings_SetIsSensor(bodyCreationSettings, true);
	this->bodyId = JPH_BodyInterface_CreateAndAddBody(this->bodyInterface,
													  bodyCreationSettings,
													  JPH_Activation_Activate);
	JPH_BodyCreationSettings_Destroy(bodyCreationSettings);
}

void CoinOnPlayerContactAdded(Actor *this, JPH_BodyId /*bodyId*/)
{
	const CoinData *data = this->extraData;
	if (!data->isBlue)
	{
		GetState()->saveData->coins++;
	} else
	{
		GetState()->saveData->blueCoins++;
		GetState()->saveData->coins += 5;
	}
	PlaySoundEffect(SOUND("sfx_coincling"));
	ActorFireOutput(this, COIN_OUTPUT_COLLECTED, PARAM_NONE);
	RemoveActor(this);
}

void CoinInit(Actor *this, const KvList *params, Transform *transform)
{
	this->extraData = calloc(1, sizeof(CoinData));
	CheckAlloc(this->extraData);
	CoinData *data = this->extraData;
	data->isBlue = KvGetBool(params, "isBlue", false);

	transform->position.y = -0.25f;
	CreateCoinSensor(this, transform);

	this->actorWall = malloc(sizeof(ActorWall));
	this->actorWall->a = v2(0, 0.125f);
	this->actorWall->b = v2(0, -0.125f);
	strncpy(this->actorWall->tex, data->isBlue ? TEXTURE("actor_bluecoin") : TEXTURE("actor_coin"), 80);
	this->actorWall->uvScale = 1.0f;
	this->actorWall->uvOffset = 0.0f;
	this->actorWall->height = 0.25f;
	ActorWallBake(this);

	this->OnPlayerContactAdded = CoinOnPlayerContactAdded;
}

void CoinUpdate(Actor *this, double /*delta*/)
{
	CoinData *data = this->extraData;
	if (GetState()->physicsFrame % 8 == 0)
	{
		data->currentAnimationFrame++;
		data->currentAnimationFrame %= 4;

		const float uvo = 0.25f * (float)data->currentAnimationFrame;
		this->actorWall->uvOffset = uvo;
	}

	// const float rotation = atan2f(GetState()->level->player.transform.position.z - this->transform.position.z,
	// 							  GetState()->level->player.transform.position.x - this->transform.position.x) +
	// 					   PIf / 2;
	// this->actorWall->a = v2(0.125f * cosf(rotation), 0.125f * sinf(rotation));
	// this->actorWall->b = v2(-0.125f * cosf(rotation), -0.125f * sinf(rotation));
}
