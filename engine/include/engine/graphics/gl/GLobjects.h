//
// Created by droc101 on 11/20/25.
//

#ifndef GAME_GLOBJECTS_H
#define GAME_GLOBJECTS_H

#include <cglm/types.h>
#include <engine/assets/ModelLoader.h>
#include <engine/assets/TextureLoader.h>
#include <engine/graphics/std140.h>
#include <engine/structs/Color.h>
#include <engine/structs/Map.h>
#include <GL/glew.h>
#include <joltc/Math/Vector3.h>
#include <stddef.h>
#include <stdint.h>

/// The maximum number of map models OpenGL can store
#define GL_MAX_MAP_MODELS 2048

typedef struct GL_Shader GL_Shader;
typedef struct GL_ComputeShader GL_ComputeShader;
typedef struct GL_Buffer GL_Buffer;
typedef struct GL_ModelBuffers GL_ModelBuffers;
typedef struct GL_SharedUniforms GL_SharedUniforms;
typedef struct GL_DebugLine GL_DebugLine;
typedef struct GL_MapModelBuffer GL_MapModelBuffer;

struct GL_Shader
{
	/// The ID of the vertex shader
	GLuint vertexShader;
	/// The ID of the fragment shader
	GLuint fragmentShader;
	/// The ID of the shader program
	GLuint program;
};

struct GL_ComputeShader
{
	GLuint computeShader;
	GLuint program;
};

struct GL_Buffer
{
	/// The vertex array object
	GLuint vertexArrayObject;
	/// The vertex buffer object
	GLuint vertexBufferObject;
	/// The element buffer object
	GLuint elementBufferObject;
};

struct GL_ModelBuffers
{
	/// The number of LODs in this buffer
	uint32_t lodCount;
	/// The number of materials in this buffer
	uint32_t materialCount;
	/// The buffers, indexed by LOD then material
	GL_Buffer **buffers;
};

struct GL_MapModelBuffer
{
	/// The map model in this buffer
	MapModel *mapModel;
	/// The OpenGL buffer storing this model
	GL_Buffer *buffer;
};

struct STD140 GL_SharedUniforms
{
	/// The model -> screen matrix
	STD140_MAT4 worldViewMatrix;
	/// The color of the fog
	STD140_COLOR fogColor;
	/// The distance from the camera at which the fog starts
	STD140_FLOAT fogStart;
	/// The distance from the camera at which the fog is fully opaque
	STD140_FLOAT fogEnd;
	/// The global light color
	STD140_VEC3 lightColor;
	/// The global light direction
	STD140_VEC3 lightDirection;
};

struct GL_DebugLine
{
	Vector3 start;
	Vector3 startColor;
	Vector3 end;
	Vector3 endColor;
};

/// Loaded textures
extern GLuint glTextures[MAX_TEXTURES];
extern int glAssetTextureMap[MAX_TEXTURES];
/// The next available texture slot
extern int glNextFreeSlot;

/// Loaded models
extern GL_ModelBuffers *glModels[MAX_MODELS];

/// Loaded map models
extern GL_MapModelBuffer *mapModels[GL_MAX_MAP_MODELS];

/// General purpose buffer
extern GL_Buffer *glBuffer;

/// Shared uniform buffer
extern GLuint sharedUniformBuffer;

/// Texture anisotropy level
extern GLfloat anisotropyLevel;

/// Number of MSAA samples
extern GLint glMsaaSamples;

/**
 * Update the anisotropic filtering level
 */
void GL_UpdateAnisotropyLevel();

/**
 * Create a shader program from assets
 * @param fragmentAsset The fragment shader asset
 * @param vertexAsset The vertex shader asset
 * @return The constructed shader or NULLPTR on error
 */
GL_Shader *GL_ConstructShader(const char *fragmentAsset, const char *vertexAsset);

GL_ComputeShader *GL_ConstructComputeShader(const char *asset);

/**
 * Create a buffer object
 * @note This should be reused as much as possible
 * @return The buffer struct
 */
GL_Buffer *GL_ConstructBuffer();

/**
 * Destroy a GL_Shader struct
 * @param shd The shader to destroy
 */
void GL_DestroyShader(GL_Shader *shd);

void GL_DestroyComputeShader(GL_ComputeShader *shd);

/**
 * Bind/use a GL shader
 */
void GL_UseShader(const GL_Shader *shd);

/**
 * Bind/use a GL buffer
 */
void GL_BindBuffer(const GL_Buffer *buffer);

/**
 * Destroy a GL_Buffer struct
 * @param buffer The buffer to destroy
 */
void GL_DestroyBuffer(GL_Buffer *buffer);

/**
 * Load and register a texture from an asset
 * @param texture The texture name
 */
void GL_LoadTextureFromAsset(const char *texture);

/**
 * Register a texture from pixel data
 * @param image The height of the texture
 * @return The slot the texture was registered in
 */
int GL_RegisterTexture(const Image *image);

/**
 * Load a model into OpenGL
 * @param model The model definition to load
 * @param lod The LOD to load
 * @param material The material to load
 */
void GL_LoadModel(const ModelDefinition *model, uint32_t lod, size_t material);

/**
 * Destroy all loaded map models
 */
void GL_DestroyMapModels();

/**
 * Initialize GL objects
 */
void GL_InitObjects();

/**
 * Destroy GL objects
 */
void GL_DestroyObjects();

/**
 * Unload all textures
 */
void GL_DeleteAllTextures();

/**
 * Unload all models
 */
void GL_DestroyAllModels();

#endif //GAME_GLOBJECTS_H
