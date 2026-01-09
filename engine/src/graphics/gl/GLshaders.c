//
// Created by droc101 on 11/20/25.
//

#include <cglm/cglm.h>
#include <engine/assets/AssetReader.h>
#include <engine/graphics/gl/GLdebug.h>
#include <engine/graphics/gl/GLobjects.h>
#include <engine/graphics/gl/GLshaders.h>
#include <stdbool.h>
#include <stddef.h>

GL_Shader *uiTexturedShader;
GL_Shader *uiColoredShader;
GL_Shader *shadedWallShader;
GL_Shader *unshadedWallShader;
GL_Shader *skyShader;
GL_Shader *modelUnshadedShader;
GL_Shader *modelShadedShader;
GL_Shader *debugShader;

GLint floorTextureLoc;
GLint floorShadeLoc;
GLint floorHeightLoc;
GLint floorSharedUniformsLoc;

GLint hudColoredColorLoc; // TODO: confusing name -- location of the color uniform in the colored shader

GLint hudTexturedTextureLoc; // TODO: confusing name -- location of the texture uniform in the textured shader
GLint hudTexturedColorLoc;
GLint hudTexturedRegionLoc;

GLint shadedWallTextureLoc;
GLint shadedWallModelWorldMatrixLoc;
GLint shadedWallSharedUniformsLoc;
GLint shadedWallTransformMatrixLoc;

GLint unshadedWallTextureLoc;
GLint unshadedWallModelWorldMatrixLoc;
GLint unshadedWallSharedUniformsLoc;
GLint unshadedWallTransformMatrixLoc;


GLint debugColorLoc;

bool GL_LoadShaders()
{
	uiTexturedShader = GL_ConstructShaderFromAssets(SHADER("gl/hud_textured_f"), SHADER("gl/hud_textured_v"));
	uiColoredShader = GL_ConstructShaderFromAssets(SHADER("gl/hud_color_f"), SHADER("gl/hud_color_v"));
	shadedWallShader = GL_ConstructShaderFromAssets(SHADER("gl/actor_wall_shaded_f"), SHADER("gl/actor_wall_shaded_v"));
	unshadedWallShader = GL_ConstructShaderFromAssets(SHADER("gl/actor_wall_unshaded_f"),
													  SHADER("gl/actor_wall_unshaded_v"));
	skyShader = GL_ConstructShaderFromAssets(SHADER("gl/sky_f"), SHADER("gl/sky_v"));
	modelShadedShader = GL_ConstructShaderFromAssets(SHADER("gl/model_shaded_f"), SHADER("gl/model_shaded_v"));
	modelUnshadedShader = GL_ConstructShaderFromAssets(SHADER("gl/model_unshaded_f"), SHADER("gl/model_unshaded_v"));
	debugShader = GL_ConstructShaderFromAssets(SHADER("gl/debug_f"), SHADER("gl/debug_v"));

	if (!uiTexturedShader ||
		!uiColoredShader ||
		!shadedWallShader ||
		!unshadedWallShader ||
		!skyShader ||
		!modelShadedShader ||
		!modelUnshadedShader ||
		!debugShader)
	{
		GL_Error("Failed to compile shaders");
		return false;
	}

	hudColoredColorLoc = glGetUniformLocation(uiColoredShader->program, "col");

	hudTexturedTextureLoc = glGetUniformLocation(uiTexturedShader->program, "alb");
	hudTexturedColorLoc = glGetUniformLocation(uiTexturedShader->program, "col");
	hudTexturedRegionLoc = glGetUniformLocation(uiTexturedShader->program, "region");

	shadedWallTextureLoc = glGetUniformLocation(shadedWallShader->program, "alb");
	shadedWallModelWorldMatrixLoc = glGetUniformLocation(shadedWallShader->program, "MODEL_WORLD_MATRIX");
	shadedWallSharedUniformsLoc = glGetUniformBlockIndex(shadedWallShader->program, "SharedUniforms");
	shadedWallTransformMatrixLoc = glGetUniformLocation(shadedWallShader->program, "transformMatrix");

	unshadedWallTextureLoc = glGetUniformLocation(unshadedWallShader->program, "alb");
	unshadedWallModelWorldMatrixLoc = glGetUniformLocation(unshadedWallShader->program, "MODEL_WORLD_MATRIX");
	unshadedWallSharedUniformsLoc = glGetUniformBlockIndex(unshadedWallShader->program, "SharedUniforms");
	unshadedWallTransformMatrixLoc = glGetUniformLocation(unshadedWallShader->program, "transformMatrix");

	debugColorLoc = glGetUniformLocation(debugShader->program, "color");

	return true;
}

void GL_DestroyShaders()
{
	GL_DestroyShader(uiTexturedShader);
	GL_DestroyShader(uiColoredShader);
	GL_DestroyShader(shadedWallShader);
	GL_DestroyShader(unshadedWallShader);
	GL_DestroyShader(skyShader);
	GL_DestroyShader(modelShadedShader);
	GL_DestroyShader(modelUnshadedShader);
	GL_DestroyShader(debugShader);
	glUseProgram(0);
	glDisableVertexAttribArray(0);
}
