//
// Created by droc101 on 1/16/26.
//

#include <engine/actor/GlobalFog.h>
#include <engine/helpers/MathEx.h>
#include <engine/physics/Physics.h>
#include <engine/structs/Actor.h>
#include <engine/structs/ActorDefinition.h>
#include <engine/structs/Color.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/KVList.h>
#include <engine/structs/Map.h>
#include <engine/subsystem/Error.h>
#include <joltc/Math/Quat.h>
#include <joltc/Math/Transform.h>
#include <joltc/Math/Vector3.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

static Actor *interpolatingActor = NULL;

static Color interpolationPreviousColor = COLOR_BLACK;
static float interpolationPreviousStart = 10.0f;
static float interpolationPreviousEnd = 20.0f;
static uint64_t interpolationStartTick = 0;

typedef struct GlobalFogData
{
	Color fogColor;
	float fogStart;
	float fogEnd;
	int interpolationTicks;
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
	data->interpolationTicks = KvGetInt(params, "interpolation_ticks", PHYSICS_TARGET_TPS);
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
		GetState()->map->changeFlags |= MAP_FOG_CHANGED;
		data->startOn = false;
	}

	if (interpolatingActor == this)
	{
		const int ticksIntoInterpolation = (int)(GetState()->map->physicsTick - interpolationStartTick);
		const float interpolationFactor = (1.0f / (float)data->interpolationTicks) * (float)ticksIntoInterpolation;
		GetState()->map->fogStart = lerp(interpolationPreviousStart, data->fogStart, interpolationFactor);
		GetState()->map->fogEnd = lerp(interpolationPreviousEnd, data->fogEnd, interpolationFactor);
		GetState()->map->fogColor.r = lerp(interpolationPreviousColor.r, data->fogColor.r, interpolationFactor);
		GetState()->map->fogColor.g = lerp(interpolationPreviousColor.g, data->fogColor.g, interpolationFactor);
		GetState()->map->fogColor.b = lerp(interpolationPreviousColor.b, data->fogColor.b, interpolationFactor);
		GetState()->map->fogColor.a = lerp(interpolationPreviousColor.a, data->fogColor.a, interpolationFactor);
		GetState()->map->changeFlags |= MAP_FOG_CHANGED;
		if (ticksIntoInterpolation == data->interpolationTicks)
		{
			interpolatingActor = NULL;
		}
	}
}

static void GlobalFogSetHandler(Actor *this, const Actor * /*sender*/, const Param * /*param*/)
{
	const GlobalFogData *data = this->extraData;
	if (data->interpolationTicks == 0)
	{
		interpolatingActor = NULL; // stop any existing interpolation, but don't start a new one
		GetState()->map->fogColor = data->fogColor;
		GetState()->map->fogStart = data->fogStart;
		GetState()->map->fogEnd = data->fogEnd;
		GetState()->map->changeFlags |= MAP_FOG_CHANGED;
	} else
	{
		interpolatingActor = this;
		interpolationStartTick = GetState()->map->physicsTick;
		interpolationPreviousColor = GetState()->map->fogColor;
		interpolationPreviousStart = GetState()->map->fogStart;
		interpolationPreviousEnd = GetState()->map->fogEnd;
	}
}

static void GlobalFogSetInstantHandler(Actor *this, const Actor * /*sender*/, const Param * /*param*/)
{
	const GlobalFogData *data = this->extraData;
	interpolatingActor = NULL; // stop any existing interpolation, but don't start a new one
	GetState()->map->fogColor = data->fogColor;
	GetState()->map->fogStart = data->fogStart;
	GetState()->map->fogEnd = data->fogEnd;
	GetState()->map->changeFlags |= MAP_FOG_CHANGED;
}

void GlobalFogDestroy(Actor *this)
{
	if (interpolatingActor == this)
	{
		interpolatingActor = NULL; // got vaporized :(
	}
}

static ActorDefinition definition = {
	.actorType = ACTOR_TYPE_GLOBAL_FOG,
	.Update = GlobalFogUpdate,
	.OnPlayerContactAdded = DefaultActorOnPlayerContactAdded,
	.OnPlayerContactPersisted = DefaultActorOnPlayerContactPersisted,
	.OnPlayerContactRemoved = DefaultActorOnPlayerContactRemoved,
	.RenderUi = DefaultActorRenderUi,
	.Destroy = GlobalFogDestroy,
	.Init = GlobalFogInit,
};

void RegisterGlobalFog()
{
	RegisterDefaultActorInputs(&definition);
	RegisterActorInput(&definition, GLOBAL_FOG_INPUT_SET, GlobalFogSetHandler);
	RegisterActorInput(&definition, GLOBAL_FOG_INPUT_SET_INSTANT, GlobalFogSetInstantHandler);
	RegisterActor(GLOBAL_FOG_ACTOR_NAME, &definition);
}
