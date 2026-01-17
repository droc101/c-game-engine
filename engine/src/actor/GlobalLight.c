//
// Created by droc101 on 1/4/26.
//

#include <engine/actor/GlobalLight.h>
#include <engine/helpers/MathEx.h>
#include <engine/physics/Physics.h>
#include <engine/structs/Actor.h>
#include <engine/structs/ActorDefinition.h>
#include <engine/structs/Color.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/KVList.h>
#include <engine/structs/Vector2.h>
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
static float interpolationPreviousPitch = 0.0f;
static float interpolationPreviousYaw = 0.0f;
static uint64_t interpolationStartTick = 0;

typedef struct GlobalLightData
{
	Vector2 lightAngle;
	Color lightColor;
	int interpolationTicks;
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
	data->interpolationTicks = KvGetInt(params, "interpolation_ticks", PHYSICS_TARGET_TPS);
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

	if (interpolatingActor == this)
	{
		const int ticksIntoInterpolation = (int)(GetState()->map->physicsTick - interpolationStartTick);
		const float interpolationFactor = (1.0f / (float)data->interpolationTicks) * (float)ticksIntoInterpolation;
		GetState()->map->lightAngle.x = lerp(interpolationPreviousPitch, data->lightAngle.x, interpolationFactor);
		GetState()->map->lightAngle.y = lerp(interpolationPreviousYaw, data->lightAngle.y, interpolationFactor);
		GetState()->map->lightColor.r = lerp(interpolationPreviousColor.r, data->lightColor.r, interpolationFactor);
		GetState()->map->lightColor.g = lerp(interpolationPreviousColor.g, data->lightColor.g, interpolationFactor);
		GetState()->map->lightColor.b = lerp(interpolationPreviousColor.b, data->lightColor.b, interpolationFactor);
		GetState()->map->lightColor.a = lerp(interpolationPreviousColor.a, data->lightColor.a, interpolationFactor);
		if (ticksIntoInterpolation == data->interpolationTicks)
		{
			interpolatingActor = NULL;
		}
	}
}

static void GlobalLightSetHandler(Actor *this, const Actor * /*sender*/, const Param * /*param*/)
{
	const GlobalLightData *data = this->extraData;
	if (data->interpolationTicks == 0)
	{
		interpolatingActor = NULL; // stop any existing interpolation, but don't start a new one
		GetState()->map->lightAngle = data->lightAngle;
		GetState()->map->lightColor = data->lightColor;
	} else
	{
		interpolatingActor = this;
		interpolationStartTick = GetState()->map->physicsTick;
		interpolationPreviousColor = GetState()->map->lightColor;
		interpolationPreviousPitch = GetState()->map->lightAngle.x;
		interpolationPreviousYaw = GetState()->map->lightAngle.y;
	}
}

static void GlobalLightSetInstantHandler(Actor *this, const Actor * /*sender*/, const Param * /*param*/)
{
	const GlobalLightData *data = this->extraData;
	interpolatingActor = NULL; // stop any existing interpolation, but don't start a new one
	GetState()->map->lightAngle = data->lightAngle;
	GetState()->map->lightColor = data->lightColor;
}

void GlobalLightDestroy(Actor *this)
{
	if (interpolatingActor == this)
	{
		interpolatingActor = NULL; // got vaporized :(
	}
}

static ActorDefinition definition = {
	.actorType = ACTOR_TYPE_GLOBAL_LIGHT,
	.Update = GlobalLightUpdate,
	.OnPlayerContactAdded = DefaultActorOnPlayerContactAdded,
	.OnPlayerContactPersisted = DefaultActorOnPlayerContactPersisted,
	.OnPlayerContactRemoved = DefaultActorOnPlayerContactRemoved,
	.RenderUi = DefaultActorRenderUi,
	.Destroy = GlobalLightDestroy,
	.Init = GlobalLightInit,
};

void RegisterGlobalLight()
{
	RegisterDefaultActorInputs(&definition);
	RegisterActorInput(&definition, GLOBAL_LIGHT_INPUT_SET, GlobalLightSetHandler);
	RegisterActorInput(&definition, GLOBAL_LIGHT_INPUT_SET_INSTANT, GlobalLightSetInstantHandler);
	RegisterActor(GLOBAL_LIGHT_ACTOR_NAME, &definition);
}
