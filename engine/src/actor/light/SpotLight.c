//
// Created by droc101 on 9/24/26.
//

#include <engine/actor/light/SpotLight.h>
#include <engine/structs/ActorDefinition.h>

#include "engine/graphics/RenderingHelpers.h"
#include "engine/structs/Actor.h"
#include "engine/structs/Light.h"

typedef struct SpotLightData
{
	DynamicLight dlight;
	bool isLightOn;
} SpotLightData;

static void SpotLightInit(Actor *this, const KvList params, const Transform *transform)
{
	ActorCreateEmptyBody(this, transform);

	this->extraData = calloc(1, sizeof(SpotLightData));
	CheckAlloc(this->extraData);
	SpotLightData *data = this->extraData;
	data->dlight.parent = this->bodyId;
	data->dlight.light.type = LIGHT_TYPE_SPOT;
	data->isLightOn = KvGetBool(params, "start_on", true);

	data->dlight.light.color = KvGetColor(params, "color", COLOR_WHITE);
	data->dlight.light.brightness = KvGetFloat(params, "brightness", 1.0f);

	data->dlight.light.constantAttenuation = KvGetFloat(params, "constant_attenuation", 0.0f);
	data->dlight.light.linearAttenuation = KvGetFloat(params, "linear_attenuation", 0.0f);
	data->dlight.light.quadraticAttenuation = KvGetFloat(params, "quadratic_attenuation", 1.0f);
	data->dlight.light.attenuationMultiplier = KvGetFloat(params, "attenuation_multiplier", 32.0f);

	data->dlight.light.brightAngle = KvGetFloat(params, "bright_angle", 45.0f);
	data->dlight.light.fadingAngle = KvGetFloat(params, "fading_angle", 60.0f);
	data->dlight.light.cookie = strdup(KvGetString(params, "cookie", ""));

	if (data->isLightOn)
	{
		AddDynamicLight(&data->dlight);
	}
}

static void SpotLightUpdate(Actor *this, double /*delta*/)
{
	SpotLightData *data = this->extraData;
}

static void SpotLightDestroy(Actor *this)
{
	SpotLightData *data = this->extraData;
	if (data->isLightOn)
	{
		RemoveDynamicLight(&data->dlight);
		data->isLightOn = false;
	}
	free(data->dlight.light.cookie);
}

static void SpotLightTurnOnHandler(Actor *this, const Actor * /*sender*/, const Param * /*param*/)
{
	SpotLightData *data = this->extraData;
	if (!data->isLightOn)
	{
		data->isLightOn = true;
		AddDynamicLight(&data->dlight);
	}
}

static void SpotLightTurnOffHandler(Actor *this, const Actor * /*sender*/, const Param * /*param*/)
{
	SpotLightData *data = this->extraData;
	if (data->isLightOn)
	{
		data->isLightOn = false;
		RemoveDynamicLight(&data->dlight);
	}
}

ActorDefinition spotLightActorDefinition = {
	.Init = SpotLightInit,
	.Update = SpotLightUpdate,
	.Destroy = SpotLightDestroy,
};

void RegisterSpotLight()
{
	RegisterDefaultActorInputs(&spotLightActorDefinition);
	RegisterActorInput(&spotLightActorDefinition, SPOT_LIGHT_INPUT_TURN_ON, SpotLightTurnOnHandler);
	RegisterActorInput(&spotLightActorDefinition, SPOT_LIGHT_INPUT_TURN_OFF, SpotLightTurnOffHandler);
	RegisterActor(SPOT_LIGHT_ACTOR_NAME, &spotLightActorDefinition);
}
