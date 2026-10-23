//
// Created by droc101 on 4/9/26.
//

#ifndef GAME_LIGHT_H
#define GAME_LIGHT_H

#include <cglm/vec3.h>
#include <stdint.h>

#define MAX_LIGHT_COUNT 128

typedef enum LightType LightType;

typedef struct Light Light;

typedef struct DynamicLight DynamicLight;

enum LightType
{
	LIGHT_TYPE_POINT,
	LIGHT_TYPE_SPOT,
	/// Do not use. Exists for compatibility with SDK tools.
	LIGHT_TYPE_AREA,
	LIGHT_TYPE_DIRECTIONAL,
};

struct Light
{
	/// The type of light
	LightType type;
	/// The world space position of the light
	Vector3 position;
	/// Euler angle rotation for the light, in degrees
	Vector3 rotation;
	/// The color of the light
	Color color;
	/// The brightness scale of the light
	float brightness;
	float constantAttenuation;
	float linearAttenuation;
	float quadraticAttenuation;
	float attenuationMultiplier;
	/// The angle at which the spotlight will retain up to 75% brightness
	float brightAngle;
	/// The angle at which the spotlight will reach 0% brightness
	float fadingAngle;
	/// The texture projected by spot lights
	char *cookie; // mmmmm tasty
};

struct DynamicLight
{
	/// The @c position and @c rotation fields are treated as being relative to that of the parent
	Light light;
	JPH_BodyID parent;
};

#endif //GAME_LIGHT_H
