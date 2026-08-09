// Include only. This file will not compile as a standalone module.

#extension GL_EXT_debug_printf : require
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_nonuniform_qualifier : require

const uint LIGHT_TYPE_POINT = 0u;
const uint LIGHT_TYPE_SPOT = 1u;
const uint LIGHT_TYPE_AREA = 2u;
const uint LIGHT_TYPE_DIRECTIONAL = 3u;

struct Light {
    uint type; // Maps to an enum in C
    vec3 position;
    vec4 rotation;
    vec3 negativeForwardDirection;
    vec3 color;
    float brightness;
    float constantAttenuation;
    float linearAttenuation;
    float quadraticAttenuation;
    float attenuationMultiplier;
    float brightAngle; // 0-90 degrees
    float fadingAngle;
    uint shadowMapIndex;
    float _padding[2];
	mat4 transformMatrix;
};

const float MIN_BRIGHTNESS = 1.0 / 256.0;
const vec2 MAGIC_XY = vec2(0.06711056, 0.00583715);
const float MAGIC_Z = 52.9829189;

const mat4 transforms[6] = {
	mat4(
         0,  0,  1,  0,
         0, -1,  0,  0,
         1,  0,  0,  0,
         0,  0,  0,  1
	),
    mat4(
         0,  0, -1,  0,
         0, -1,  0,  0,
        -1,  0,  0,  0,
         0,  0,  0,  1
	),
	mat4(
         1,  0,  0,  0,
         0,  0,  1,  0,
         0, -1,  0,  0,
         0,  0,  0,  1
	),
	mat4(
         1,  0,  0,  0,
         0,  0, -1,  0,
         0,  1,  0,  0,
         0,  0,  0,  1
	),
	mat4(
         1,  0,  0,  0,
         0, -1,  0,  0,
         0,  0, -1,  0,
         0,  0,  0,  1
	),
	mat4(
        -1,  0,  0,  0,
         0, -1,  0,  0,
         0,  0,  1,  0,
         0,  0,  0,  1
	),
};