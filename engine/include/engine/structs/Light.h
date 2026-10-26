//
// Created by droc101 on 4/9/26.
//

/**
 * @file Light.h
 * @c Light and @c DynamicLight definitions
 */

#ifndef GAME_LIGHT_H
#define GAME_LIGHT_H

#include <cglm/vec3.h>

#define MAX_LIGHT_COUNT 128

typedef struct Light Light;

typedef struct DynamicLight DynamicLight;

/// Light type
typedef enum LightType
{
	/// Omnidirectional light
	LIGHT_TYPE_POINT,
	/// Spotlight
	LIGHT_TYPE_SPOT,
	/// @warning Do not use. Exists for compatibility with SDK tools.
	LIGHT_TYPE_AREA,
	/**
	 * Directional light, like a sun
	 * @warning Do not use on dynamic lights
	 */
	LIGHT_TYPE_DIRECTIONAL,
} LightType;

/// A realtime light
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

/**
 * A movable @c Light
 * @note The @c position and @c rotation fields of the @c Light are treated as being relative to that of the parent
 */
struct DynamicLight
{
	/// The light to control
	Light light;
	/// The physics body this light belongs to
	JPH_BodyID parent;
};

#endif //GAME_LIGHT_H
