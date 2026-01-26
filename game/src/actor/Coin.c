//
// Created by droc101 on 7/11/2024.
//

#include "actor/Coin.h"
#include <cglm/types.h>
#include <engine/assets/AssetReader.h>
#include <engine/physics/Physics.h>
#include <engine/structs/Actor.h>
#include <engine/structs/ActorDefinition.h>
#include <engine/structs/ActorWall.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/KVList.h>
#include <engine/structs/Map.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/SoundSystem.h>
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

typedef struct CoinData
{
	bool isBlue;
	uint8_t currentAnimationFrame;
} CoinData;

static inline void CreateCoinSensor(Actor *this, const Transform *transform)
{
	JPH_Shape *shape = (JPH_Shape *)JPH_BoxShape_Create((Vector3[]){{0.25f, 0.25f, 0.25f}}, JPH_DefaultConvexRadius);
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
	const float rotation = atan2f(GetState()->map->player.transform.position.z - position.z,
								  GetState()->map->player.transform.position.x - position.x) +
						   GLM_PI_2f;
	this->actorWall->a = v2(0.125f * cosf(rotation), 0.125f * sinf(rotation));
	this->actorWall->b = v2(-0.125f * cosf(rotation), -0.125f * sinf(rotation));
	ActorWallBake(this);
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
	(void)PlaySound(SOUND("sfx/coincling"), SOUND_CATEGORY_SFX);
	ActorFireOutput(this, COIN_OUTPUT_COLLECTED, PARAM_NONE);
	RemoveActor(this);
}

static void CoinInit(Actor *this, const KvList params, Transform *transform)
{
	this->extraData = calloc(1, sizeof(CoinData));
	CheckAlloc(this->extraData);
	CoinData *data = this->extraData;
	data->isBlue = KvGetBool(params, "isBlue", false);

	const Transform adjustedTransform = {
		.position.x = transform->position.x,
		.position.y = transform->position.y,
		.position.z = transform->position.z,
		.rotation.w = 1.0f,
	};
	CreateCoinSensor(this, &adjustedTransform);

	this->actorWall = malloc(sizeof(ActorWall));
	CheckAlloc(this->actorWall);
	this->actorWall->a = v2(0, 0.125f);
	this->actorWall->b = v2(0, -0.125f);
	this->actorWall->tex = malloc(strlen(TEXTURE("actor/bluecoin")) + 1);
	strcpy(this->actorWall->tex, data->isBlue ? TEXTURE("actor/bluecoin") : TEXTURE("actor/coin"));
	this->actorWall->uvScale = 1.0f;
	this->actorWall->uvOffset = 0.0f;
	this->actorWall->height = 0.25f;
	this->actorWall->unshaded = false;
	ActorWallBake(this);
}

ActorDefinition coinActorDefinition = {
	.Update = CoinUpdate,
	.OnPlayerContactAdded = CoinOnPlayerContactAdded,
	.OnPlayerContactPersisted = DefaultActorOnPlayerContactPersisted,
	.OnPlayerContactRemoved = DefaultActorOnPlayerContactRemoved,
	.RenderUi = DefaultActorRenderUi,
	.Destroy = DefaultActorDestroy,
	.Init = CoinInit,
};

void RegisterCoin()
{
	RegisterDefaultActorInputs(&coinActorDefinition);
	RegisterActor(COIN_ACTOR_NAME, &coinActorDefinition);
}
