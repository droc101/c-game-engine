//
// Created by droc101 on 11/20/25.
//

#ifndef GAME_GLSHADERS_H
#define GAME_GLSHADERS_H

#include <engine/graphics/gl/GLobjects.h>
#include <GL/glew.h>
#include <stdbool.h>

/// The shader used for textured UI
extern GL_Shader *uiTexturedShader;
/// The shader used for untextured UI
extern GL_Shader *uiColoredShader;
/// The shader used for actor walls with shading
extern GL_Shader *actorWallShadedShader;
/// The shader used for actor walls without shading
extern GL_Shader *actorWallUnshadedShader;
/// The shader used for the sky
extern GL_Shader *skyShader;
/// The shader used for generic models without shading
extern GL_Shader *modelUnshadedShader;
/// The shader used for generic models with shading
extern GL_Shader *modelShadedShader;
/// The shader used for the debug renderer
extern GL_Shader *debugShader;

#pragma region Uniform/Attribute Locations

/// The location of the color uniform in the UI colored shader
extern GLint uiColoredColorLoc;
/// The location of the vertex position attribute in the UI colored shader
extern GLint uiColoredVertexLoc;

/// The location of the texture uniform in the UI textured shader
extern GLint uiTexturedTextureLoc;
/// The location of the color uniform in the UI textured shader
extern GLint uiTexturedColorLoc;
/// The location of the region uniform in the UI textured shader
extern GLint uiTexturedRegionLoc;
/// The location of the vertex position attribute in the UI textured shader
extern GLint uiTexturedVertexLoc;
/// The location of the vertex UV attribute in the UI textured shader
extern GLint uiTexturedUvLoc;

/// The location of the texture uniform in the shaded actor wall shader
extern GLint actorWallShadedTextureLoc;
/// The location of the model -> world matrix in the shaded actor wall shader
extern GLint actorWallShadedModelWorldMatrixLoc;
/// The location of the shared uniforms buffer in the shaded actor wall shader
extern GLint actorWallShadedSharedUniformsLoc;
/// The location of the transform matrix uniform in the shaded actor wall shader
extern GLint actorWallShadedTransformMatrixLoc;
/// The location of the vertex position attribute in the shaded actor wall shader
extern GLint actorWallShadedVertexLoc;
/// The location of the vertex UV attribute in the shaded actor wall shader
extern GLint actorWallShadedUvLoc;
/// The location of the vertex angle attribute in the shaded actor wall shader
extern GLint actorWallShadedAngleLoc;

/// The location of the texture uniform in the unshaded actor wall shader
extern GLint actorWallUnshadedTextureLoc;
/// The location of the model -> world matrix in the unshaded actor wall shader
extern GLint actorWallUnshadedModelWorldMatrixLoc;
/// The location of the shared uniforms buffer in the unshaded actor wall shader
extern GLint actorWallUnshadedSharedUniformsLoc;
/// The location of the transform matrix uniform in the unshaded actor wall shader
extern GLint actorWallUnshadedTransformMatrixLoc;
/// The location of the vertex position attribute in the unshaded actor wall shader
extern GLint actorWallUnshadedVertexLoc;
/// The location of the vertex UV attribute in the unshaded actor wall shader
extern GLint actorWallUnshadedUvLoc;

/// The location of the color uniform in the debug shader
extern GLint debugColorLoc;
/// The location of the shaded uniforms buffer on the debug shader
extern GLint debugSharedUniformsLoc;
/// The location of the vertex position attribute on the debug shader
extern GLint debugVertexLoc;

/// The location of the shared uniforms buffer on the sky shader
extern GLint skySharedUniformsLoc;
/// The location of the model -> world matrix on the shy shader
extern GLint skyModelWorldMatrixLoc;
/// The location of the vertex position attribute in the sky shader
extern GLint skyVertexLoc;
/// The location of the vertex UV attribute in the sky shader
extern GLint skyUvLoc;
/// The location of the vertex color attribute in the sky shader
extern GLint skyColorLoc;

/// The location of the shared uniforms buffer on the shaded model shader
extern GLint shadedModelSharedUniformsLoc;
/// The location of the model -> world matrix on the shaded model shader
extern GLint shadedModelModelWorldMatrixLoc;
/// The location of the material color uniform in the shaded model shader
extern GLint shadedModelAlbColorLoc;
/// The location of the modifier color uniform in the shaded model shader
extern GLint shadedModelModColorLoc;
/// The location of the vertex position attribute in the shaded model shader
extern GLint shadedModelVertexLoc;
/// The location of the vertex UV attribute in the shaded model shader
extern GLint shadedModelUvLoc;
/// The location of the vertex normal attribute in the shaded model shader
extern GLint shadedModelNormalLoc;
/// The location of the vertex color attribute in the shaded model shader
extern GLint shadedModelColorLoc;

/// The location of the shared uniforms buffer on the unshaded model shader
extern GLint unshadedModelSharedUniformsLoc;
/// The location of the model -> world matrix on the unshaded model shader
extern GLint unshadedModelModelWorldMatrixLoc;
/// The location of the material color uniform in the unshaded model shader
extern GLint unshadedModelAlbColorLoc;
/// The location of the modifier color uniform in the unshaded model shader
extern GLint unshadedModelModColorLoc;
/// The location of the vertex position attribute in the unshaded model shader
extern GLint unshadedModelVertexLoc;
/// The location of the vertex UV attribute in the unshaded model shader
extern GLint unshadedModelUvLoc;
/// The location of the vertex color attribute in the unshaded model shader
extern GLint unshadedModelColorLoc;

#pragma endregion

/**
 * Load shader uniform locations
 */
bool GL_LoadShaders();

/**
 * Destroy GL shaders
 */
void GL_DestroyShaders();

#endif //GAME_GLSHADERS_H
