//
// Created by droc101 on 11/11/24.
//
// This file is only to be included by OpenGL rendering code.
// Do not include it from outside the "/Helpers/Graphics/GL" folder.
//

#ifndef GLINTERNAL_H
#define GLINTERNAL_H

#include <cglm/cglm.h>
#include <GL/glew.h>
#include "../../../defines.h"

typedef struct GL_Shader GL_Shader;
typedef struct GL_Buffer GL_Buffer;
typedef struct GL_ModelBuffers GL_ModelBuffers;
typedef struct GL_WallBuffers GL_WallBuffers;
typedef struct GL_SharedUniforms GL_SharedUniforms;

struct GL_Shader
{
	/// The ID of the vertex shader
	GLuint vertexShader;
	/// The ID of the fragment shader
	GLuint fragmentShader;
	/// The ID of the shader program
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
	size_t lodCount;
	/// The number of materials in this buffer
	size_t materialCount;
	/// The buffers, indexed by LOD then material
	GL_Buffer **buffers;
};

struct GL_WallBuffers
{
	/// The number of walls in this buffer
	size_t wallCount;
	/// The texture name for this buffer
	char texture[80];
	/// The GPU buffer for the walls
	GL_Buffer *buffer;
};

struct __attribute__((aligned(16))) GL_SharedUniforms
{
	/// The model -> screen matrix
	mat4 worldViewMatrix;
	/// The color of the fog
	vec3 fogColor;
	/// The distance from the camera at which the fog starts
	float fogStart;
	/// The distance from the camera at which the fog is fully opaque
	float fogEnd;
	/// The yaw of the camera
	float cameraYaw;
};

/// The maximum number of different wall textures that can be used in a level
#define GL_MAX_WALL_BUFFERS 128
/// The maximum number of walls that can be in a single wall buffer (i.e. a single texture)
#define GL_MAX_WALLS_PER_BUFFER 2048
/**
 * Log an OpenGL error
 * @param error the error message
 */
void GL_Error(const char *error);

/**
 * Create a shader program from assets
 * @param fsh The fragment shader asset
 * @param vsh The vertex shader asset
 * @return The constructed shader or NULLPTR on error
 */
GL_Shader *GL_ConstructShaderFromAssets(const char *fsh, const char *vsh);

/**
 * Create a shader program
 * @param fsh The fragment shader source
 * @param vsh The vertex shader source
 * @return The shader struct or NULLPTR on error
 */
GL_Shader *GL_ConstructShader(const char *fsh, const char *vsh);

/**
 * Create a buffer object
 * @note This should be reused as much as possible
 * @return The buffer struct
 */
GL_Buffer *GL_ConstructBuffer();

/**
 * Debug message callback for OpenGL
 * @param source The source of the message
 * @param type The type of the message
 * @param id The ID of the message
 * @param severity The severity of the message
 * @param length The length of the message
 * @param msg The message
 * @param data Extra data
 */
void GL_DebugMessageCallback(GLenum source,
							 GLenum type,
							 GLuint id,
							 GLenum severity,
							 GLsizei length,
							 const GLchar *msg,
							 const void *data);


/**
 * Load and register a texture from an asset
 * @param texture The texture name
 */
void GL_LoadTextureFromAsset(const char *texture);

/**
 * Draw an actor wall in 3D
 * @param actor The actor to draw
 * @note This expects 3D mode to be enabled
 */
void GL_DrawActorWall(const Actor *actor);

/**
 * Register a texture from pixel data
 * @param pixelData The raw RGBA8 pixel data
 * @param width The width of the texture
 * @param height The height of the texture
 * @return The slot the texture was registered in
 */
int GL_RegisterTexture(const unsigned char *pixelData, int width, int height);

/**
 * Draw a textured rectangle to the screen
 * @param pos The position in pixels
 * @param size The size in pixels
 * @param texture The texture name
 * @param regionStart The start of the region in pixels
 * @param regionEnd The end of the region in pixels
 * @param color The modulate color
 */
void GL_DrawTexture_Internal(Vector2 pos,
							 Vector2 size,
							 const char *texture,
							 Color color,
							 Vector2 regionStart,
							 Vector2 regionEnd);

/**
 * Set the level parameters for rendering
 * @param mvp The model -> screen matrix
 * @param l The level
 */
void GL_SetLevelParams(mat4 *mvp, const Level *l);

/**
 * Enable 3D mode
 */
void GL_Enable3D();

/**
 * Disable 3D mode
 */
void GL_Disable3D();

/**
 * Destroy any existing wall buffers.
 * This is OK to call if there are no buffers.
 */
void GL_DestroyWallBuffers();

/**
 * Load shader uniform locations
 */
void LoadShaderLocations();

/**
 * Destroy a GL_Shader struct
 * @param shd The shader to destroy
 */
void GL_DestroyShader(GL_Shader *shd);

/**
 * Destroy a GL_Buffer struct
 * @param buffer The buffer to destroy
 */
void GL_DestroyBuffer(GL_Buffer *buffer);

/**
 * Load a model into OpenGL
 * @param model The model definition to load
 * @param lod The LOD to load
 * @param material The material to load
 */
void GL_LoadModel(const ModelDefinition *model, uint lod, int material);

/**
 * Get the wall buffer for a texture, or create it if it doesn't exist.
 * @param texture The texture name
 * @return The wall buffer for the texture
 * @warning This will crash the game if there are no free slots left.
 */
GL_WallBuffers *GL_GetWallBuffer(const char *texture);

/**
 * Render a single material of a model
 * @param model The model to render
 * @param modelWorldMatrix The model -> world matrix
 * @param lod The level of detail to render
 * @param material The material to render
 * @param skin The skin to use for the model
 */
void GL_RenderModelPart(const ModelDefinition *model, const mat4 modelWorldMatrix, uint lod, int material, int skin);


#endif //GLINTERNAL_H
