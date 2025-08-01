//
// Created by droc101 on 7/24/25.
//

#ifndef SHADERLOADER_H
#define SHADERLOADER_H

#include "../../../defines.h"

#define SHADER_ASSET_VERSION 1

typedef enum ShaderPlatform ShaderPlatform;
typedef enum ShaderType ShaderType;

typedef struct Shader Shader;

enum ShaderPlatform
{
	PLATFORM_OPENGL,
	PLATFORM_VULKAN
};

enum ShaderType
{
	SHADER_TYPE_FRAG,
	SHADER_TYPE_VERT
};

struct Shader
{
	ShaderPlatform platform;
	ShaderType type;
	size_t glslLength;
	char* glsl;
	size_t spirvLength;
	uint32_t *spirv;
};

Shader *LoadShader(const char *asset);

void FreeShader(Shader *shader);

#endif //SHADERLOADER_H
