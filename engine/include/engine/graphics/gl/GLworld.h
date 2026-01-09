//
// Created by droc101 on 11/20/25.
//

#ifndef GAME_GLWORLD_H
#define GAME_GLWORLD_H

#include <cglm/types.h>
#include <engine/assets/ModelLoader.h>
#include <engine/graphics/gl/GLobjects.h>
#include <engine/structs/Actor.h>
#include <engine/structs/Camera.h>
#include <engine/structs/Color.h>
#include <engine/structs/Map.h>
#include <GL/glew.h>
#include <stddef.h>
#include <stdint.h>

/**
 * Get the transformation matrix for a camera
 * @param camera The camera
 * @param modelViewProjectionMatrix
 * @return A mat4 MODEL_VIEW_PROJECTION matrix of the camera (World space to screen space)
 */
void GL_GetMatrix(const Camera *camera, mat4 *modelViewProjectionMatrix);

/**
 * Get the transform matrix for the viewmodel/held item
 * @param out The destination matrix
 */
void GL_GetViewmodelMatrix(mat4 *out);

/**
 * OpenGL code to render the 3D portion of a map
 * @param map The map to render
 * @param camera The camera to render with
 * @note - This does not render the sky
 * @note - This destroys the contents of the depth buffer
 */
void GL_RenderMap(const Map *map, const Camera *camera);

/**
 * Render a 3D model
 * @param model The model to render
 * @param modelWorldMatrix The model -> world matrix
 * @param skin The skin to use
 * @param lod The lod to use
 * @param modColor
 */
void GL_RenderModel(const ModelDefinition *model,
					const mat4 modelWorldMatrix,
					uint32_t skin,
					uint32_t lod,
					Color modColor);

/**
 * Render a map model
 * @param model The map model to render
 */
void GL_RenderMapModel(const GL_MapModelBuffer *model);

/**
 * Load a map into OpenGL
 * @param map the map to load
 */
void GL_LoadMap(const Map *map);

/**
 * Render a single material of a model
 * @param model The model to render
 * @param modelWorldMatrix The model -> world matrix
 * @param lod The level of detail to render
 * @param material The material to render
 * @param skin The skin to use for the model
 * @param modColor
 */
void GL_RenderModelPart(const ModelDefinition *model,
						const mat4 modelWorldMatrix,
						uint32_t lod,
						size_t material,
						uint32_t skin,
						Color modColor);


/**
 * Draw a shaded actor wall in 3D
 * @param actor The actor to draw
 * @param actorXfm
 * @note This expects 3D mode to be enabled
 */
void GL_DrawShadedActorWall(const Actor *actor, const mat4 actorXfm);

/**
 * Draw an unshaded actor wall in 3D
 * @param actor The actor to draw
 * @param actorXfm
 * @note This expects 3D mode to be enabled
 */
void GL_DrawUnshadedActorWall(const Actor *actor, const mat4 actorXfm);

/**
 * Set the map parameters for rendering
 * @param modelViewProjection The model -> screen matrix
 * @param map The map
 */
void GL_SetMapParams(mat4 *modelViewProjection, const Map *map);

/**
 * Enable 3D mode
 */
void GL_Enable3D(void);

/**
 * Disable 3D mode
 */
void GL_Disable3D();

#endif //GAME_GLWORLD_H
