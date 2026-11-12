//
// Created by droc101 on 7/24/25.
//

/**
 * @file ShaderLoader.h
 * Shader asset loader
 */

#ifndef SHADERLOADER_H
#define SHADERLOADER_H

#include <stddef.h>
#include <stdint.h>

/// Shader asset version number
#define SHADER_ASSET_VERSION 2

typedef struct Shader Shader;

/// The pipeline phase a shader is intended for
typedef enum ShaderType
{
	/// Fragment shader
	SHADER_TYPE_FRAG,
	/// Vertex shader
	SHADER_TYPE_VERT,
	/// Compute shader
	SHADER_TYPE_COMP,
	/// Geometry shader
	SHADER_TYPE_GEOM,
} ShaderType;

/// A loaded shader asset
struct Shader
{
	/// The type of this shader
	ShaderType type;
	/// The length of the SPIRV code (in number of @code uint32_t@endcode s
	size_t spirvLength;
	/// The SPIRV code
	uint32_t *spirv;
};

/**
 * Load a shader from a path
 * @param asset The path of the shader
 * @return The @c Shader, or @c NULL on error
 * @private
 */
Shader *LoadShader(const char *asset);

/**
 * Free a shader
 * @param shader The shader to free
 * @private
 */
void FreeShader(Shader *shader);

#endif //SHADERLOADER_H
