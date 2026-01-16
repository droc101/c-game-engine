//
// Created by droc101 on 1/16/26.
//

#include <engine/actor/GlobalFog.h>
#include <engine/structs/Actor.h>
#include <engine/structs/ActorDefinition.h>
#include <engine/structs/Color.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/KVList.h>
#include <engine/subsystem/Error.h>
#include <joltc/Math/Quat.h>
#include <joltc/Math/Transform.h>
#include <joltc/Math/Vector3.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct GlobalFogData
{
	Color fogColor;
	float fogStart;
	float fogEnd;
	bool startOn;
} GlobalFogData;

void GlobalFogInit(Actor *this, const KvList params, Transform *transform)
{
	this->extraData = calloc(1, sizeof(GlobalFogData));
	CheckAlloc(this->extraData);
	GlobalFogData *data = this->extraData;
	Vector3 euler;
	JPH_Quat_GetEulerAngles(&transform->rotation, &euler);
	data->fogColor = KvGetColor(params, "fog_color", COLOR_WHITE);
	data->fogStart = KvGetFloat(params, "fog_start", 10.0f);
	data->fogEnd = KvGetFloat(params, "fog_end", 20.0f);
	data->startOn = KvGetBool(params, "start_on", true);
}

static void GlobalFogUpdate(Actor *this, double /*delta*/)
{
	GlobalFogData *data = this->extraData;
	if (data->startOn)
	{
		GetState()->map->fogColor = data->fogColor;
		GetState()->map->fogStart = data->fogStart;
		GetState()->map->fogEnd = data->fogEnd;
		data->startOn = false;
	}
}

static void GlobalFogSetHandler(Actor *this, const Actor * /*sender*/, const Param * /*param*/)
{
	const GlobalFogData *data = this->extraData;
	GetState()->map->fogColor = data->fogColor;
	GetState()->map->fogStart = data->fogStart;
	GetState()->map->fogEnd = data->fogEnd;
}

static ActorDefinition definition = {
	.actorType = ACTOR_TYPE_GLOBAL_FOG,
	.Update = GlobalFogUpdate,
	.OnPlayerContactAdded = DefaultActorOnPlayerContactAdded,
	.OnPlayerContactPersisted = DefaultActorOnPlayerContactPersisted,
	.OnPlayerContactRemoved = DefaultActorOnPlayerContactRemoved,
	.RenderUi = DefaultActorRenderUi,
	.Destroy = DefaultActorDestroy,
	.Init = GlobalFogInit,
};

void RegisterGlobalFog()
{
	RegisterDefaultActorInputs(&definition);
	RegisterActorInput(&definition, GLOBAL_FOG_INPUT_SET, GlobalFogSetHandler);
	RegisterActor(GLOBAL_FOG_ACTOR_NAME, &definition);
}
