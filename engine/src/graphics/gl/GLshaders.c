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

GLint uiColoredColorLoc = 0;
GLint uiColoredVertexLoc = 0;

GLint uiTexturedVertexLoc = 0;
GLint uiTexturedUvLoc = 1;
GLint uiTexturedTextureLoc = 0;
GLint uiTexturedColorLoc = 1;
GLint uiTexturedRegionLoc = 2;

GLint actorWallShadedVertexLoc = 0;
GLint actorWallShadedUvLoc = 1;
GLint actorWallShadedNormalLoc = 2;
GLint actorWallShadedSharedUniformsLoc = 0;
GLint actorWallShadedLightMetadataLoc = 1;
GLint actorWallShadedPointLightsLoc = 2;
GLint actorWallShadedTransformMatrixLoc = 0;
GLint actorWallShadedTextureLoc = 1;
GLint actorWallShadedAlbColorLoc = 2;

GLint actorWallUnshadedVertexLoc = 0;
GLint actorWallUnshadedUvLoc = 1;
GLint actorWallUnshadedSharedUniformsLoc = 0;
GLint actorWallUnshadedTransformMatrixLoc = 0;
GLint actorWallUnshadedTextureLoc = 1;
GLint actorWallUnshadedAlbColorLoc = 2;

GLint debugColorLoc = 1;
GLint debugVertexLoc = 0;
GLint debugSharedUniformsLoc = 0;

GLint skyVertexLoc = 0;
GLint skyColorLoc = 1;
GLint skyUvLoc = 2;
GLint skySharedUniformsLoc = 0;
GLint skyModelWorldMatrixLoc = 0;

GLint shadedActorModelVertexLoc = 0;
GLint shadedActorModelUvLoc = 1;
GLint shadedActorModelColorLoc = 2;
GLint shadedActorModelNormalLoc = 3;
GLint shadedActorModelSharedUniformsLoc = 0;
GLint shadedActorLightMetadataLoc = 1;
GLint shadedActorPointLightsLoc = 2;
GLint shadedActorModelAlbColorLoc = 1;
GLint shadedActorModelModColorLoc = 2;
GLint shadedActorModelModelWorldMatrixLoc = 3;

GLint unshadedActorModelVertexLoc = 0;
GLint unshadedActorModelUvLoc = 1;
GLint unshadedActorModelColorLoc = 2;
GLint unshadedActorModelSharedUniformsLoc = 0;
GLint unshadedActorModelModelWorldMatrixLoc = 0;
GLint unshadedActorModelAlbColorLoc = 2;
GLint unshadedActorModelModColorLoc = 3;

GLint shadedMapModelVertexLoc = 0;
GLint shadedMapModelUvLoc = 1;
GLint shadedMapModelUv2Loc = 2;
GLint shadedMapModelSharedUniformsLoc = 0;
GLint shadedMapModelModelWorldMatrixLoc = 0;
GLint shadedMapModelLightmapLoc = 3;

GLint unshadedMapModelVertexLoc = 0;
GLint unshadedMapModelUvLoc = 1;
GLint unshadedMapModelSharedUniformsLoc = 0;
GLint unshadedMapModelModelWorldMatrixLoc = 0;

GLint tonemapVertexLoc = 0;
GLint tonemapUvLoc = 1;
GLint tonemapFramebufferLoc = 0;
GLint tonemapExposureLoc = 1;

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
		LogError("OpenGL: Failed to compile shaders\n");
		return false;
	}

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
