//
// Created by droc101 on 7/11/2024.
//

#include "Coin.h"
#include <cglm/types.h>
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
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "../Helpers/Core/AssetReader.h"
#include "../Helpers/Core/Error.h"
#include "../Helpers/Core/KVList.h"
#include "../Helpers/Core/Physics/Physics.h"
#include "../Helpers/Core/SoundSystem.h"
#include "../Structs/Actor.h"
#include "../Structs/ActorDefinition.h"
#include "../Structs/GlobalState.h"
#include "../Structs/Level.h"
#include "../Structs/Param.h"
#include "../Structs/Vector2.h"
#include "../Structs/Wall.h"

typedef struct CoinData
{
	bool isBlue;
	uint8_t currentAnimationFrame;
} CoinData;

static inline void CreateCoinSensor(Actor *this, const Transform *transform)
{
	const JPH_Shape *shape = (const JPH_Shape *)JPH_BoxShape_Create((Vector3[]){{0.25f, 0.25f, 0.25f}},
																	JPH_DefaultConvexRadius);
	JPH_BodyCreationSettings *bodyCreationSettings = JPH_BodyCreationSettings_Create2_GAME(shape,
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

static void CoinUpdate(Actor *this, double /*delta*/)
{
	CoinData *data = this->extraData;
	if (GetState()->physicsFrame % 8 == 0)
	{
		data->currentAnimationFrame++;
		data->currentAnimationFrame %= 4;

		const float uvo = 0.25f * (float)data->currentAnimationFrame;
		this->actorWall->uvOffset = uvo;
	}

	Vector3 position = {};
	JPH_BodyInterface_GetPosition(this->bodyInterface, this->bodyId, &position);
	const float rotation = atan2f(GetState()->level->player.transform.position.z - position.z,
								  GetState()->level->player.transform.position.x - position.x) +
						   GLM_PI_2f;
	this->actorWall->a = v2(0.125f * cosf(rotation), 0.125f * sinf(rotation));
	this->actorWall->b = v2(-0.125f * cosf(rotation), -0.125f * sinf(rotation));
}

static void CoinOnPlayerContactAdded(Actor *this, JPH_BodyId /*bodyId*/)
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
	(void)PlaySoundEffect(SOUND("sfx/coincling"), 0, 1, NULL, NULL);
	ActorFireOutput(this, COIN_OUTPUT_COLLECTED, PARAM_NONE);
	RemoveActor(this);
}

static void CoinInit(Actor *this, const KvList params, Transform *transform)
{
	this->extraData = calloc(1, sizeof(CoinData));
	CheckAlloc(this->extraData);
	CoinData *data = this->extraData;
	data->isBlue = KvGetBool(params, "isBlue", false);

	transform->position.y = -0.25f;
	transform->rotation.y = 0;
	CreateCoinSensor(this, transform);

	this->actorWall = malloc(sizeof(ActorWall));
	this->actorWall->a = v2(0, 0.125f);
	this->actorWall->b = v2(0, -0.125f);
	strncpy(this->actorWall->tex, data->isBlue ? TEXTURE("actor/bluecoin") : TEXTURE("actor/coin"), 80);
	this->actorWall->uvScale = 1.0f;
	this->actorWall->uvOffset = 0.0f;
	this->actorWall->height = 0.25f;
	ActorWallBake(this);
}

static ActorDefinition definition = {.actorType = ACTOR_TYPE_COIN,
									 .Update = CoinUpdate,
									 .OnPlayerContactAdded = CoinOnPlayerContactAdded,
									 .OnPlayerContactPersisted = DefaultActorOnPlayerContactPersisted,
									 .OnPlayerContactRemoved = DefaultActorOnPlayerContactRemoved,
									 .RenderUi = DefaultActorRenderUi,
									 .Destroy = DefaultActorDestroy,
									 .Init = CoinInit};

void RegisterCoin()
{
	RegisterDefaultActorInputs(&definition);
	RegisterActor(COIN_ACTOR_NAME, &definition);
}
