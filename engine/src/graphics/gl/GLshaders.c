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
GL_Shader *actorWallShadedShader;
GL_Shader *actorWallUnshadedShader;
GL_Shader *skyShader;
GL_Shader *modelUnshadedShader;
GL_Shader *modelShadedShader;
GL_Shader *debugShader;

GLint uiColoredColorLoc;
GLint uiColoredVertexLoc;

GLint uiTexturedTextureLoc;
GLint uiTexturedColorLoc;
GLint uiTexturedRegionLoc;
GLint uiTexturedVertexLoc;
GLint uiTexturedUvLoc;

GLint actorWallShadedTextureLoc;
GLint actorWallShadedModelWorldMatrixLoc;
GLint actorWallShadedSharedUniformsLoc;
GLint actorWallShadedTransformMatrixLoc;
GLint actorWallShadedVertexLoc;
GLint actorWallShadedUvLoc;
GLint actorWallShadedAngleLoc;

GLint actorWallUnshadedTextureLoc;
GLint actorWallUnshadedModelWorldMatrixLoc;
GLint actorWallUnshadedSharedUniformsLoc;
GLint actorWallUnshadedTransformMatrixLoc;
GLint actorWallUnshadedVertexLoc;
GLint actorWallUnshadedUvLoc;

GLint debugColorLoc;
GLint debugSharedUniformsLoc;
GLint debugVertexLoc;

GLint skySharedUniformsLoc;
GLint skyModelWorldMatrixLoc;
GLint skyVertexLoc;
GLint skyUvLoc;
GLint skyColorLoc;

GLint shadedModelSharedUniformsLoc;
GLint shadedModelModelWorldMatrixLoc;
GLint shadedModelAlbColorLoc;
GLint shadedModelModColorLoc;
GLint shadedModelVertexLoc;
GLint shadedModelUvLoc;
GLint shadedModelNormalLoc;
GLint shadedModelColorLoc;

GLint unshadedModelSharedUniformsLoc;
GLint unshadedModelModelWorldMatrixLoc;
GLint unshadedModelAlbColorLoc;
GLint unshadedModelModColorLoc;
GLint unshadedModelVertexLoc;
GLint unshadedModelUvLoc;
GLint unshadedModelColorLoc;

bool GL_LoadShaders()
{
	uiTexturedShader = GL_ConstructShaderFromAssets(SHADER("gl/hud_textured_f"), SHADER("gl/hud_textured_v"));
	uiColoredShader = GL_ConstructShaderFromAssets(SHADER("gl/hud_color_f"), SHADER("gl/hud_color_v"));
	actorWallShadedShader = GL_ConstructShaderFromAssets(SHADER("gl/actor_wall_shaded_f"),
														 SHADER("gl/actor_wall_shaded_v"));
	actorWallUnshadedShader = GL_ConstructShaderFromAssets(SHADER("gl/actor_wall_unshaded_f"),
														   SHADER("gl/actor_wall_unshaded_v"));
	skyShader = GL_ConstructShaderFromAssets(SHADER("gl/sky_f"), SHADER("gl/sky_v"));
	modelShadedShader = GL_ConstructShaderFromAssets(SHADER("gl/model_shaded_f"), SHADER("gl/model_shaded_v"));
	modelUnshadedShader = GL_ConstructShaderFromAssets(SHADER("gl/model_unshaded_f"), SHADER("gl/model_unshaded_v"));
	debugShader = GL_ConstructShaderFromAssets(SHADER("gl/debug_f"), SHADER("gl/debug_v"));

	if (!uiTexturedShader ||
		!uiColoredShader ||
		!actorWallShadedShader ||
		!actorWallUnshadedShader ||
		!skyShader ||
		!modelShadedShader ||
		!modelUnshadedShader ||
		!debugShader)
	{
		GL_Error("Failed to compile shaders");
		return false;
	}

	uiColoredColorLoc = glGetUniformLocation(uiColoredShader->program, "col");
	uiColoredVertexLoc = glGetAttribLocation(uiColoredShader->program, "VERTEX");

	uiTexturedTextureLoc = glGetUniformLocation(uiTexturedShader->program, "alb");
	uiTexturedColorLoc = glGetUniformLocation(uiTexturedShader->program, "col");
	uiTexturedRegionLoc = glGetUniformLocation(uiTexturedShader->program, "region");
	uiTexturedVertexLoc = glGetAttribLocation(uiTexturedShader->program, "VERTEX");
	uiTexturedUvLoc = glGetAttribLocation(uiTexturedShader->program, "VERTEX_UV");

	actorWallShadedTextureLoc = glGetUniformLocation(actorWallShadedShader->program, "alb");
	actorWallShadedModelWorldMatrixLoc = glGetUniformLocation(actorWallShadedShader->program, "MODEL_WORLD_MATRIX");
	actorWallShadedSharedUniformsLoc = glGetUniformBlockIndex(actorWallShadedShader->program, "SharedUniforms");
	actorWallShadedTransformMatrixLoc = glGetUniformLocation(actorWallShadedShader->program, "transformMatrix");
	actorWallShadedVertexLoc = glGetAttribLocation(actorWallShadedShader->program, "VERTEX");
	actorWallShadedUvLoc = glGetAttribLocation(actorWallShadedShader->program, "VERTEX_UV");
	actorWallShadedAngleLoc = glGetAttribLocation(actorWallShadedShader->program, "VERTEX_ANGLE");

	actorWallUnshadedTextureLoc = glGetUniformLocation(actorWallUnshadedShader->program, "alb");
	actorWallUnshadedModelWorldMatrixLoc = glGetUniformLocation(actorWallUnshadedShader->program, "MODEL_WORLD_MATRIX");
	actorWallUnshadedSharedUniformsLoc = glGetUniformBlockIndex(actorWallUnshadedShader->program, "SharedUniforms");
	actorWallUnshadedTransformMatrixLoc = glGetUniformLocation(actorWallUnshadedShader->program, "transformMatrix");
	actorWallUnshadedVertexLoc = glGetAttribLocation(actorWallUnshadedShader->program, "VERTEX");
	actorWallUnshadedUvLoc = glGetAttribLocation(actorWallUnshadedShader->program, "VERTEX_UV");

	debugColorLoc = glGetUniformLocation(debugShader->program, "color");
	debugSharedUniformsLoc = glGetUniformBlockIndex(debugShader->program, "SharedUniforms");
	debugVertexLoc = glGetAttribLocation(debugShader->program, "VERTEX");

	skySharedUniformsLoc = glGetUniformBlockIndex(skyShader->program, "SharedUniforms");
	skyModelWorldMatrixLoc = glGetUniformLocation(skyShader->program, "MODEL_WORLD_MATRIX");
	skyVertexLoc = glGetAttribLocation(skyShader->program, "VERTEX");
	skyUvLoc = glGetAttribLocation(skyShader->program, "VERTEX_UV");
	skyColorLoc = glGetAttribLocation(skyShader->program, "VERTEX_COLOR");

	shadedModelSharedUniformsLoc = glGetUniformBlockIndex(modelShadedShader->program, "SharedUniforms");
	shadedModelModelWorldMatrixLoc = glGetUniformLocation(modelShadedShader->program, "MODEL_WORLD_MATRIX");
	shadedModelAlbColorLoc = glGetUniformLocation(modelShadedShader->program, "albColor");
	shadedModelModColorLoc = glGetUniformLocation(modelShadedShader->program, "modColor");
	shadedModelVertexLoc = glGetAttribLocation(modelShadedShader->program, "VERTEX");
	shadedModelUvLoc = glGetAttribLocation(modelShadedShader->program, "VERTEX_UV");
	shadedModelNormalLoc = glGetAttribLocation(modelShadedShader->program, "VERTEX_NORMAL");
	shadedModelColorLoc = glGetAttribLocation(modelShadedShader->program, "VERTEX_COLOR");

	unshadedModelSharedUniformsLoc = glGetUniformBlockIndex(modelUnshadedShader->program, "SharedUniforms");
	unshadedModelModelWorldMatrixLoc = glGetUniformLocation(modelUnshadedShader->program, "MODEL_WORLD_MATRIX");
	unshadedModelAlbColorLoc = glGetUniformLocation(modelUnshadedShader->program, "albColor");
	unshadedModelModColorLoc = glGetUniformLocation(modelUnshadedShader->program, "modColor");
	unshadedModelVertexLoc = glGetAttribLocation(modelUnshadedShader->program, "VERTEX");
	unshadedModelUvLoc = glGetAttribLocation(modelUnshadedShader->program, "VERTEX_UV");
	unshadedModelColorLoc = glGetAttribLocation(modelUnshadedShader->program, "VERTEX_COLOR");

	return true;
}

void GL_DestroyShaders()
{
	GL_DestroyShader(uiTexturedShader);
	GL_DestroyShader(uiColoredShader);
	GL_DestroyShader(actorWallShadedShader);
	GL_DestroyShader(actorWallUnshadedShader);
	GL_DestroyShader(skyShader);
	GL_DestroyShader(modelShadedShader);
	GL_DestroyShader(modelUnshadedShader);
	GL_DestroyShader(debugShader);
	glUseProgram(0);
	glDisableVertexAttribArray(0);
}
