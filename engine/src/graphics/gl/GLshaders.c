//
// Created by droc101 on 11/20/25.
//

#include <cglm/cglm.h>
#include <engine/assets/AssetReader.h>
#include <engine/graphics/gl/GLobjects.h>
#include <engine/graphics/gl/GLshaders.h>
#include <engine/subsystem/Logging.h>
#include <stdbool.h>
#include <stddef.h>

GL_Shader *uiTexturedShader;
GL_Shader *uiColoredShader;
GL_Shader *actorWallShadedShader;
GL_Shader *actorWallUnshadedShader;
GL_Shader *skyShader;
GL_Shader *actorUnshadedShader;
GL_Shader *actorShadedShader;
GL_Shader *debugShader;
GL_Shader *mapShadedShader;
GL_Shader *mapUnshadedShader;
GL_Shader *tonemapShader;

GLint uiColoredColorLoc;
GLint uiColoredVertexLoc;

GLint uiTexturedTextureLoc;
GLint uiTexturedColorLoc;
GLint uiTexturedRegionLoc;
GLint uiTexturedVertexLoc;
GLint uiTexturedUvLoc;

GLint actorWallShadedLightMetadataLoc;
GLint actorWallShadedPointLightsLoc;
GLint actorWallShadedTextureLoc;
GLint actorWallShadedModelWorldMatrixLoc;
GLint actorWallShadedSharedUniformsLoc;
GLint actorWallShadedTransformMatrixLoc;
GLint actorWallShadedAlbColorLoc;
GLint actorWallShadedVertexLoc;
GLint actorWallShadedUvLoc;
GLint actorWallShadedNormalLoc;

GLint actorWallUnshadedTextureLoc;
GLint actorWallUnshadedModelWorldMatrixLoc;
GLint actorWallUnshadedSharedUniformsLoc;
GLint actorWallUnshadedTransformMatrixLoc;
GLint actorWallUnshadedAlbColorLoc;
GLint actorWallUnshadedVertexLoc;
GLint actorWallUnshadedUvLoc;

GLint debugSharedUniformsLoc;
GLint debugColorLoc;
GLint debugVertexLoc;

GLint skySharedUniformsLoc;
GLint skyModelWorldMatrixLoc;
GLint skyVertexLoc;
GLint skyUvLoc;
GLint skyColorLoc;

GLint shadedActorModelSharedUniformsLoc;
GLint shadedActorLightMetadataLoc;
GLint shadedActorPointLightsLoc;
GLint shadedActorModelModelWorldMatrixLoc;
GLint shadedActorModelAlbColorLoc;
GLint shadedActorModelModColorLoc;
GLint shadedActorModelVertexLoc;
GLint shadedActorModelUvLoc;
GLint shadedActorModelNormalLoc;
GLint shadedActorModelColorLoc;

GLint unshadedActorModelSharedUniformsLoc;
GLint unshadedActorModelModelWorldMatrixLoc;
GLint unshadedActorModelAlbColorLoc;
GLint unshadedActorModelModColorLoc;
GLint unshadedActorModelVertexLoc;
GLint unshadedActorModelUvLoc;
GLint unshadedActorModelColorLoc;

GLint shadedMapModelSharedUniformsLoc;
GLint shadedMapModelModelWorldMatrixLoc;
GLint shadedMapModelLightmapLoc;
GLint shadedMapModelVertexLoc;
GLint shadedMapModelUvLoc;
GLint shadedMapModelUv2Loc;

GLint unshadedMapModelSharedUniformsLoc;
GLint unshadedMapModelModelWorldMatrixLoc;
GLint unshadedMapModelVertexLoc;
GLint unshadedMapModelUvLoc;

GLint tonemapFramebufferLoc;
GLint tonemapExposureLoc;
GLint tonemapVertexLoc;
GLint tonemapUvLoc;

bool GL_LoadShaders()
{
	uiTexturedShader = GL_ConstructShader(SHADER("gl/hud/textured_f"), SHADER("gl/hud/textured_v"));
	uiColoredShader = GL_ConstructShader(SHADER("gl/hud/colored_f"), SHADER("gl/hud/colored_v"));
	actorWallShadedShader = GL_ConstructShader(SHADER("gl/actor/wall_shaded_f"), SHADER("gl/actor/wall_shaded_v"));
	actorWallUnshadedShader = GL_ConstructShader(SHADER("gl/actor/wall_unshaded_f"),
												 SHADER("gl/actor/wall_unshaded_v"));
	skyShader = GL_ConstructShader(SHADER("gl/map/sky_f"), SHADER("gl/map/sky_v"));
	actorShadedShader = GL_ConstructShader(SHADER("gl/actor/actor_shaded_f"), SHADER("gl/actor/actor_shaded_v"));
	actorUnshadedShader = GL_ConstructShader(SHADER("gl/actor/actor_unshaded_f"), SHADER("gl/actor/actor_unshaded_v"));
	debugShader = GL_ConstructShader(SHADER("gl/debug_f"), SHADER("gl/debug_v"));
	mapShadedShader = GL_ConstructShader(SHADER("gl/map/map_shaded_f"), SHADER("gl/map/map_shaded_v"));
	mapUnshadedShader = GL_ConstructShader(SHADER("gl/map/map_unshaded_f"), SHADER("gl/map/map_unshaded_v"));
	tonemapShader = GL_ConstructShader(SHADER("gl/tonemap_f"), SHADER("gl/tonemap_v"));

	if (!uiTexturedShader ||
		!uiColoredShader ||
		!actorWallShadedShader ||
		!actorWallUnshadedShader ||
		!skyShader ||
		!actorShadedShader ||
		!actorUnshadedShader ||
		!debugShader ||
		!mapShadedShader ||
		!mapUnshadedShader ||
		!tonemapShader)
	{
		LogError("OpenGL: Failed to compile shaders");
		return false;
	}

	uiColoredColorLoc = glGetUniformLocation(uiColoredShader->program, "col");
	uiColoredVertexLoc = glGetAttribLocation(uiColoredShader->program, "VERTEX");

	uiTexturedTextureLoc = glGetUniformLocation(uiTexturedShader->program, "alb");
	uiTexturedColorLoc = glGetUniformLocation(uiTexturedShader->program, "col");
	uiTexturedRegionLoc = glGetUniformLocation(uiTexturedShader->program, "region");
	uiTexturedVertexLoc = glGetAttribLocation(uiTexturedShader->program, "VERTEX");
	uiTexturedUvLoc = glGetAttribLocation(uiTexturedShader->program, "VERTEX_UV");

	actorWallShadedLightMetadataLoc = glGetUniformBlockIndex(actorWallShadedShader->program, "LightMetadata");
	actorWallShadedPointLightsLoc =
			2; //glGetProgramResourceIndex(actorWallShadedShader->program, GL_SHADER_STORAGE_BLOCK, "PointLights");
	actorWallShadedTextureLoc = glGetUniformLocation(actorWallShadedShader->program, "alb");
	actorWallShadedModelWorldMatrixLoc = glGetUniformLocation(actorWallShadedShader->program, "MODEL_WORLD_MATRIX");
	actorWallShadedSharedUniformsLoc = glGetUniformBlockIndex(actorWallShadedShader->program, "SharedUniforms");
	actorWallShadedTransformMatrixLoc = glGetUniformLocation(actorWallShadedShader->program, "transformMatrix");
	actorWallShadedAlbColorLoc = glGetUniformLocation(actorWallShadedShader->program, "albColor");
	actorWallShadedVertexLoc = glGetAttribLocation(actorWallShadedShader->program, "VERTEX");
	actorWallShadedUvLoc = glGetAttribLocation(actorWallShadedShader->program, "VERTEX_UV");
	actorWallShadedNormalLoc = glGetAttribLocation(actorWallShadedShader->program, "VERTEX_NORMAL");

	actorWallUnshadedTextureLoc = glGetUniformLocation(actorWallUnshadedShader->program, "alb");
	actorWallUnshadedModelWorldMatrixLoc = glGetUniformLocation(actorWallUnshadedShader->program, "MODEL_WORLD_MATRIX");
	actorWallUnshadedSharedUniformsLoc = glGetUniformBlockIndex(actorWallUnshadedShader->program, "SharedUniforms");
	actorWallUnshadedTransformMatrixLoc = glGetUniformLocation(actorWallUnshadedShader->program, "transformMatrix");
	actorWallUnshadedAlbColorLoc = glGetUniformLocation(actorWallUnshadedShader->program, "albColor");
	actorWallUnshadedVertexLoc = glGetAttribLocation(actorWallUnshadedShader->program, "VERTEX");
	actorWallUnshadedUvLoc = glGetAttribLocation(actorWallUnshadedShader->program, "VERTEX_UV");

	debugSharedUniformsLoc = glGetUniformBlockIndex(debugShader->program, "SharedUniforms");
	debugVertexLoc = glGetAttribLocation(debugShader->program, "VERTEX");
	debugColorLoc = glGetAttribLocation(debugShader->program, "VERTEX_COLOR");

	skySharedUniformsLoc = glGetUniformBlockIndex(skyShader->program, "SharedUniforms");
	skyModelWorldMatrixLoc = glGetUniformLocation(skyShader->program, "MODEL_WORLD_MATRIX");
	skyVertexLoc = glGetAttribLocation(skyShader->program, "VERTEX");
	skyUvLoc = glGetAttribLocation(skyShader->program, "VERTEX_UV");
	skyColorLoc = glGetAttribLocation(skyShader->program, "VERTEX_COLOR");

	shadedActorLightMetadataLoc = glGetUniformBlockIndex(actorShadedShader->program, "LightMetadata");
	shadedActorPointLightsLoc =
			2; //glGetProgramResourceIndex(actorShadedShader->program, GL_SHADER_STORAGE_BLOCK, "PointLights");
	shadedActorModelSharedUniformsLoc = glGetUniformBlockIndex(actorShadedShader->program, "SharedUniforms");
	shadedActorModelModelWorldMatrixLoc = glGetUniformLocation(actorShadedShader->program, "MODEL_WORLD_MATRIX");
	shadedActorModelAlbColorLoc = glGetUniformLocation(actorShadedShader->program, "albColor");
	shadedActorModelModColorLoc = glGetUniformLocation(actorShadedShader->program, "modColor");
	shadedActorModelVertexLoc = glGetAttribLocation(actorShadedShader->program, "VERTEX");
	shadedActorModelUvLoc = glGetAttribLocation(actorShadedShader->program, "VERTEX_UV");
	shadedActorModelNormalLoc = glGetAttribLocation(actorShadedShader->program, "VERTEX_NORMAL");
	shadedActorModelColorLoc = glGetAttribLocation(actorShadedShader->program, "VERTEX_COLOR");

	unshadedActorModelSharedUniformsLoc = glGetUniformBlockIndex(actorUnshadedShader->program, "SharedUniforms");
	unshadedActorModelModelWorldMatrixLoc = glGetUniformLocation(actorUnshadedShader->program, "MODEL_WORLD_MATRIX");
	unshadedActorModelAlbColorLoc = glGetUniformLocation(actorUnshadedShader->program, "albColor");
	unshadedActorModelModColorLoc = glGetUniformLocation(actorUnshadedShader->program, "modColor");
	unshadedActorModelVertexLoc = glGetAttribLocation(actorUnshadedShader->program, "VERTEX");
	unshadedActorModelUvLoc = glGetAttribLocation(actorUnshadedShader->program, "VERTEX_UV");
	unshadedActorModelColorLoc = glGetAttribLocation(actorUnshadedShader->program, "VERTEX_COLOR");

	shadedMapModelSharedUniformsLoc = glGetUniformBlockIndex(mapShadedShader->program, "SharedUniforms");
	shadedMapModelModelWorldMatrixLoc = glGetUniformLocation(mapShadedShader->program, "MODEL_WORLD_MATRIX");
	shadedMapModelLightmapLoc = glGetUniformLocation(mapShadedShader->program, "lightmap");
	shadedMapModelVertexLoc = glGetAttribLocation(mapShadedShader->program, "VERTEX");
	shadedMapModelUvLoc = glGetAttribLocation(mapShadedShader->program, "VERTEX_UV");
	shadedMapModelUv2Loc = glGetAttribLocation(mapShadedShader->program, "VERTEX_LIGHTMAP_UV");

	unshadedMapModelSharedUniformsLoc = glGetUniformBlockIndex(mapUnshadedShader->program, "SharedUniforms");
	unshadedMapModelModelWorldMatrixLoc = glGetUniformLocation(mapUnshadedShader->program, "MODEL_WORLD_MATRIX");
	unshadedMapModelVertexLoc = glGetAttribLocation(mapUnshadedShader->program, "VERTEX");
	unshadedMapModelUvLoc = glGetAttribLocation(mapUnshadedShader->program, "VERTEX_UV");

	tonemapFramebufferLoc = glGetUniformLocation(tonemapShader->program, "framebuffer");
	tonemapExposureLoc = glGetUniformLocation(tonemapShader->program, "exposure");
	tonemapVertexLoc = glGetAttribLocation(tonemapShader->program, "VERTEX");
	tonemapUvLoc = glGetAttribLocation(tonemapShader->program, "VERTEX_UV");

	return true;
}

void GL_DestroyShaders()
{
	GL_DestroyShader(uiTexturedShader);
	GL_DestroyShader(uiColoredShader);
	GL_DestroyShader(actorWallShadedShader);
	GL_DestroyShader(actorWallUnshadedShader);
	GL_DestroyShader(skyShader);
	GL_DestroyShader(actorShadedShader);
	GL_DestroyShader(actorUnshadedShader);
	GL_DestroyShader(mapShadedShader);
	GL_DestroyShader(mapUnshadedShader);
	GL_DestroyShader(debugShader);
	GL_DestroyShader(tonemapShader);
	glUseProgram(0);
	glDisableVertexAttribArray(0);
}
