//
// Created by droc101 on 11/20/25.
//

#ifndef GAME_GLSHADERS_H
#define GAME_GLSHADERS_H

#include <engine/graphics/gl/GLobjects.h>
#include <GL/glew.h>
#include <stdbool.h>

extern GL_Shader *uiTexturedShader;
extern GL_Shader *uiColoredShader;
extern GL_Shader *shadedWallShader;
extern GL_Shader *unshadedWallShader;
extern GL_Shader *skyShader;
extern GL_Shader *modelUnshadedShader;
extern GL_Shader *modelShadedShader;
extern GL_Shader *debugShader;

extern GLint floorTextureLoc;
extern GLint floorShadeLoc;
extern GLint floorHeightLoc;
extern GLint floorSharedUniformsLoc;

extern GLint hudColoredColorLoc; // TODO: confusing name -- location of the color uniform in the colored shader

extern GLint hudTexturedTextureLoc; // TODO: confusing name -- location of the texture uniform in the textured shader
extern GLint hudTexturedColorLoc;
extern GLint hudTexturedRegionLoc;

extern GLint shadedWallTextureLoc;
extern GLint shadedWallModelWorldMatrixLoc;
extern GLint shadedWallSharedUniformsLoc;
extern GLint shadedWallTransformMatrixLoc;

extern GLint unshadedWallTextureLoc;
extern GLint unshadedWallModelWorldMatrixLoc;
extern GLint unshadedWallSharedUniformsLoc;
extern GLint unshadedWallTransformMatrixLoc;

extern GLint debugColorLoc;

/**
 * Load shader uniform locations
 */
bool GL_LoadShaders();

/**
 * Destroy GL shaders
 */
void GL_DestroyShaders();

#endif //GAME_GLSHADERS_H
