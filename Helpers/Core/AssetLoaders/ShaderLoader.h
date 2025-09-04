//
// Created by droc101 on 7/24/25.
//

#ifndef SHADERLOADER_H
#define SHADERLOADER_H

#include <stddef.h>
#include <stdint.h>

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
	/// Fragment shader
	SHADER_TYPE_FRAG,
	/// Vertex shader
	SHADER_TYPE_VERT
};

struct Shader
{
	/// The rendering platform of this shader
	ShaderPlatform platform;
	/// The type of this shader
	ShaderType type;
	/// The length of the GLSL in this shader
	size_t glslLength;
	/// The GLSL source code of this shader
	char *glsl;
	/// The length of the SPIRV code (in number of @code uint32_t@endcode s
	size_t spirvLength;
	/// The SPIRV code
	uint32_t *spirv;
};

/**
 * Load a shader from a path
 * @param asset The path of the shader
 * @return The @c Shader, or @c NULL on error
 */
Shader *LoadShader(const char *asset);

/**
 * Free a shader
 * @param shader The shader to free
 */
void FreeShader(Shader *shader);

#endif //SHADERLOADER_H
