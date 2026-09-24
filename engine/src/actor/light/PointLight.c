//
// Created by droc101 on 9/24/26.
//

#include <engine/actor/light/PointLight.h>
#include <engine/structs/ActorDefinition.h>
#include <engine/structs/Actor.h>
#include <engine/structs/Light.h>

#include "engine/graphics/RenderingHelpers.h"

typedef struct PointLightData
{
	DynamicLight dlight;
	bool isLightOn;
	bool wantLightOn;
} PointLightData;

static void PointLightInit(Actor *this, const KvList params, const Transform *transform)
{
	ActorCreateEmptyBody(this, transform);

	this->extraData = calloc(1, sizeof(PointLightData));
	CheckAlloc(this->extraData);
	PointLightData *data = this->extraData;
	data->dlight.parent = this->bodyId;
	data->dlight.light.type = LIGHT_TYPE_POINT;
	data->wantLightOn = KvGetBool(params, "start_on", true);

	data->dlight.light.color = KvGetColor(params, "color", COLOR_WHITE);
	data->dlight.light.brightness = KvGetFloat(params, "brightness", 1.0f);

	data->dlight.light.constantAttenuation = KvGetFloat(params, "constant_attenuation", 0.0f);
	data->dlight.light.linearAttenuation = KvGetFloat(params, "linear_attenuation", 0.0f);
	data->dlight.light.quadraticAttenuation = KvGetFloat(params, "quadratic_attenuation", 1.0f);
	data->dlight.light.attenuationMultiplier = KvGetFloat(params, "attenuation_multiplier", 32.0f);
}

static void PointLightUpdate(Actor *this, double /*delta*/)
{
	PointLightData *data = this->extraData;
	if (!data->isLightOn && data->wantLightOn)
	{
		data->isLightOn = true;
		AddDynamicLight(&data->dlight);
	} else if (data->isLightOn && !data->wantLightOn)
	{
		data->isLightOn = false;
		RemoveDynamicLight(&data->dlight);
	}
}

static void PointLightDestroy(Actor *this)
{
	PointLightData *data = this->extraData;
	if (data->isLightOn)
	{
		RemoveDynamicLight(&data->dlight);
		data->isLightOn = false;
	}
}

static void PointLightTurnOnHandler(Actor *this, const Actor * /*sender*/, const Param * /*param*/)
{
	PointLightData *data = this->extraData;
	data->wantLightOn = true;
}

static void PointLightTurnOffHandler(Actor *this, const Actor * /*sender*/, const Param * /*param*/)
{
	PointLightData *data = this->extraData;
	data->wantLightOn = false;
}

ActorDefinition pointLightActorDefinition = {
	.Init = PointLightInit,
	.Update = PointLightUpdate,
	.Destroy = PointLightDestroy,
};

void RegisterPointLight()
{
	RegisterDefaultActorInputs(&pointLightActorDefinition);
	RegisterActorInput(&pointLightActorDefinition, POINT_LIGHT_INPUT_TURN_ON, PointLightTurnOnHandler);
	RegisterActorInput(&pointLightActorDefinition, POINT_LIGHT_INPUT_TURN_OFF, PointLightTurnOffHandler);
	RegisterActor(POINT_LIGHT_ACTOR_NAME, &pointLightActorDefinition);
}
