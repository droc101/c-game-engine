//
// Created by droc101 on 1/4/26.
//

#include <engine/actor/GlobalLight.h>
#include <engine/structs/Actor.h>
#include <engine/structs/ActorDefinition.h>
#include <engine/structs/Color.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/KVList.h>
#include <engine/structs/Param.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Error.h>
#include <joltc/Math/Quat.h>
#include <joltc/Math/Transform.h>
#include <joltc/Math/Vector3.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct GlobalLightData
{
	Vector2 lightAngle;
	Color lightColor;
	bool startOn;
} GlobalLightData;

void GlobalLightInit(Actor *this, const KvList params, Transform *transform)
{
	this->extraData = calloc(1, sizeof(GlobalLightData));
	CheckAlloc(this->extraData);
	GlobalLightData *data = this->extraData;
	Vector3 euler;
	JPH_Quat_GetEulerAngles(&transform->rotation, &euler);
	data->lightAngle.x = euler.x;
	data->lightAngle.y = euler.y;
	data->lightColor = KvGetColor(params, "light_color", COLOR_WHITE);
	data->startOn = KvGetBool(params, "start_on", true);
}

static void GlobalLightUpdate(Actor *this, double /*delta*/)
{
	GlobalLightData *data = this->extraData;
	if (data->startOn)
	{
		GetState()->map->lightAngle = data->lightAngle;
		GetState()->map->lightColor = data->lightColor;
		data->startOn = false;
	}
}

static void GlobalLightSetHandler(Actor *this, const Actor * /*sender*/, const Param * /*param*/)
{
	const GlobalLightData *data = this->extraData;
	GetState()->map->lightAngle = data->lightAngle;
	GetState()->map->lightColor = data->lightColor;
}

static ActorDefinition definition = {
	.actorType = ACTOR_TYPE_GLOBAL_LIGHT,
	.Update = GlobalLightUpdate,
	.OnPlayerContactAdded = DefaultActorOnPlayerContactAdded,
	.OnPlayerContactPersisted = DefaultActorOnPlayerContactPersisted,
	.OnPlayerContactRemoved = DefaultActorOnPlayerContactRemoved,
	.RenderUi = DefaultActorRenderUi,
	.Destroy = DefaultActorDestroy,
	.Init = GlobalLightInit,
};

void RegisterGlobalLight()
{
	RegisterDefaultActorInputs(&definition);
	RegisterActorInput(&definition, GLOBAL_LIGHT_INPUT_SET, GlobalLightSetHandler);
	RegisterActor(GLOBAL_LIGHT_ACTOR_NAME, &definition);
}
