//
// Created by droc101 on 3/20/26.
//

#include <engine/actor/env/TonemapController.h>
#include <engine/helpers/MathEx.h>
#include <engine/physics/Physics.h>
#include <engine/structs/Actor.h>
#include <engine/structs/ActorDefinition.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/KVList.h>
#include <engine/structs/Map.h>
#include <engine/subsystem/Error.h>
#include <joltc/Math/Transform.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

static Actor *interpolatingActor = NULL;

static float interpolationPreviousExposure = 0.0f;
static uint64_t interpolationStartTick = 0;

typedef struct TonemapControllerData
{
	float exposure;
	int interpolationTicks;
	bool startOn;
} TonemapControllerData;

void TonemapControllerInit(Actor *this, const KvList params, Transform * /*transform*/)
{
	this->extraData = calloc(1, sizeof(TonemapControllerData));
	CheckAlloc(this->extraData);
	TonemapControllerData *data = this->extraData;
	data->exposure = KvGetFloat(params, "exposure", 1.0f);
	data->interpolationTicks = KvGetInt(params, "interpolation_ticks", PHYSICS_TARGET_TPS);
	data->startOn = KvGetBool(params, "start_on", true);
}

static void TonemapControllerUpdate(Actor *this, double /*delta*/)
{
	TonemapControllerData *data = this->extraData;
	if (data->startOn)
	{
		GetState()->map->exposure = data->exposure;
		data->startOn = false;
	}

	if (interpolatingActor == this)
	{
		const int ticksIntoInterpolation = (int)(GetState()->map->physicsTick - interpolationStartTick);
		const float interpolationFactor = (1.0f / (float)data->interpolationTicks) * (float)ticksIntoInterpolation;
		GetState()->map->exposure = lerp(interpolationPreviousExposure, data->exposure, interpolationFactor);
		if (ticksIntoInterpolation == data->interpolationTicks)
		{
			interpolatingActor = NULL;
		}
	}
}

static void TonemapControllerSetHandler(Actor *this, const Actor * /*sender*/, const Param * /*param*/)
{
	const TonemapControllerData *data = this->extraData;
	if (data->interpolationTicks == 0)
	{
		interpolatingActor = NULL; // stop any existing interpolation, but don't start a new one
		GetState()->map->exposure = data->exposure;
	} else
	{
		interpolatingActor = this;
		interpolationStartTick = GetState()->map->physicsTick;
		interpolationPreviousExposure = GetState()->map->exposure;
	}
}

static void TonemapControllerSetInstantHandler(Actor *this, const Actor * /*sender*/, const Param * /*param*/)
{
	const TonemapControllerData *data = this->extraData;
	interpolatingActor = NULL; // stop any existing interpolation, but don't start a new one
	GetState()->map->exposure = data->exposure;
}

void TonemapControllerDestroy(Actor *this)
{
	if (interpolatingActor == this)
	{
		interpolatingActor = NULL; // got vaporized :(
	}
}

ActorDefinition tonemapControllerActorDefinition = {
	.Update = TonemapControllerUpdate,
	.OnPlayerContactAdded = DefaultActorOnPlayerContactAdded,
	.OnPlayerContactPersisted = DefaultActorOnPlayerContactPersisted,
	.OnPlayerContactRemoved = DefaultActorOnPlayerContactRemoved,
	.RenderUi = DefaultActorRenderUi,
	.Interact = DefaultActorInteract,
	.Destroy = TonemapControllerDestroy,
	.Init = TonemapControllerInit,
};

void RegisterTonemapController()
{
	RegisterDefaultActorInputs(&tonemapControllerActorDefinition);
	RegisterActorInput(&tonemapControllerActorDefinition, TONEMAP_CONTROLLER_INPUT_SET, TonemapControllerSetHandler);
	RegisterActorInput(&tonemapControllerActorDefinition,
					   TONEMAP_CONTROLLER_INPUT_SET_INSTANT,
					   TonemapControllerSetInstantHandler);
	RegisterActor(TONEMAP_CONTROLLER_ACTOR_NAME, &tonemapControllerActorDefinition);
}
