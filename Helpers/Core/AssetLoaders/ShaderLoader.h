//
// Created by droc101 on 7/24/25.
//

#ifndef SHADERLOADER_H
#define SHADERLOADER_H

#include "../../../defines.h"

typedef enum ShaderPlatform: uint8_t
{
	PLATFORM_OPENGL,
	PLATFORM_VULKAN
} ShaderPlatform;

typedef enum ShaderType: uint8_t
{
	SHADER_TYPE_FRAG,
	SHADER_TYPE_VERT
} ShaderType;

typedef struct Shader
{
	ShaderPlatform platform;
	ShaderType type;
	size_t glslLength;
	char* glsl;
	size_t spirvLength;
	uint32_t *spirv;
} Shader;

Shader *LoadShader(const char *asset);

void FreeShader(Shader *shader);

#endif //SHADERLOADER_H
