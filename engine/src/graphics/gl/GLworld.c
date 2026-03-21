//
// Created by droc101 on 11/20/25.
//

#include <cglm/cglm.h>
#include <engine/assets/AssetReader.h>
#include <engine/assets/ModelLoader.h>
#include <engine/graphics/gl/GLdebug.h>
#include <engine/graphics/gl/GLframe.h>
#include <engine/graphics/gl/GLobjects.h>
#include <engine/graphics/gl/GLshaders.h>
#include <engine/graphics/gl/GLworld.h>
#include <engine/graphics/RenderingHelpers.h>
#include <engine/helpers/MathEx.h>
#include <engine/physics/Physics.h>
#include <engine/structs/Actor.h>
#include <engine/structs/ActorWall.h>
#include <engine/structs/Camera.h>
#include <engine/structs/Color.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/List.h>
#include <engine/structs/Map.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Error.h>
#include <joltc/joltc.h>
#include <joltc/Math/Quat.h>
#include <joltc/Math/RMat44.h>
#include <joltc/Math/Vector3.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

GLuint lightmap = 0;

void GL_DrawShadedActorWall(const Actor *actor, const mat4 actorXfm)
{
	const ActorWall *wall = actor->wall;

	GL_UseShader(actorWallShadedShader);

	glBindBufferBase(GL_UNIFORM_BUFFER, actorWallShadedSharedUniformsLoc, sharedUniformBuffer);

	glUniformMatrix4fv(actorWallShadedTransformMatrixLoc, 1, GL_FALSE, *actorXfm);

	glUniform4fv(actorWallShadedAlbColorLoc, 1, COLOR_TO_ARR(actor->modColor));

	glActiveTexture(GL_TEXTURE0);
	GL_LoadTextureFromAsset(wall->tex);

	const float halfHeight = wall->height / 2.0f;
	const Vector2 startVertex = v2(wall->a.x, wall->a.y);
	const Vector2 endVertex = v2(wall->b.x, wall->b.y);
	const Vector2 startUV = v2(wall->uvOffset.x, wall->uvOffset.y);
	const Vector2 endUV = v2(wall->uvScale.x * wall->length + wall->uvOffset.x,
							 wall->uvScale.y * wall->height + wall->uvOffset.y);
	const float backfaceWallAngle = wall->angle + PIf;
	const float vertices[8][6] = {
		// X Y Z U V A
		{
			startVertex.x,
			halfHeight,
			startVertex.y,
			startUV.x,
			startUV.y,
			wall->angle,
		},
		{
			endVertex.x,
			halfHeight,
			endVertex.y,
			endUV.x,
			startUV.y,
			wall->angle,
		},
		{
			endVertex.x,
			-halfHeight,
			endVertex.y,
			endUV.x,
			endUV.y,
			wall->angle,
		},
		{
			startVertex.x,
			-halfHeight,
			startVertex.y,
			startUV.x,
			endUV.y,
			wall->angle,
		},

		// backface
		{
			startVertex.x,
			halfHeight,
			startVertex.y,
			endUV.x,
			startUV.y,
			backfaceWallAngle,
		},
		{
			endVertex.x,
			halfHeight,
			endVertex.y,
			startUV.x,
			startUV.y,
			backfaceWallAngle,
		},
		{
			endVertex.x,
			-halfHeight,
			endVertex.y,
			startUV.x,
			endUV.y,
			backfaceWallAngle,
		},
		{
			startVertex.x,
			-halfHeight,
			startVertex.y,
			endUV.x,
			endUV.y,
			backfaceWallAngle,
		},
	};

	const uint32_t indices[] = {2, 1, 0, 3, 2, 0, 4, 5, 6, 4, 6, 7};

	GL_BindBuffer(glBuffer);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STREAM_DRAW);

	glVertexAttribPointer(actorWallShadedVertexLoc, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void *)0);
	glEnableVertexAttribArray(actorWallShadedVertexLoc);

	glVertexAttribPointer(actorWallShadedUvLoc,
						  2,
						  GL_FLOAT,
						  GL_FALSE,
						  6 * sizeof(GLfloat),
						  (void *)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(actorWallShadedUvLoc);

	glVertexAttribPointer(actorWallShadedAngleLoc,
						  1,
						  GL_FLOAT,
						  GL_FALSE,
						  6 * sizeof(GLfloat),
						  (void *)(5 * sizeof(GLfloat)));
	glEnableVertexAttribArray(actorWallShadedAngleLoc);

	glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, NULL);
}

void GL_DrawUnshadedActorWall(const Actor *actor, const mat4 actorXfm)
{
	const ActorWall *wall = actor->wall;

	GL_UseShader(actorWallUnshadedShader);

	glBindBufferBase(GL_UNIFORM_BUFFER, actorWallUnshadedSharedUniformsLoc, sharedUniformBuffer);

	glUniformMatrix4fv(actorWallUnshadedTransformMatrixLoc, 1, GL_FALSE, *actorXfm);

	glUniform4fv(actorWallUnshadedAlbColorLoc, 1, COLOR_TO_ARR(actor->modColor));

	glActiveTexture(GL_TEXTURE0);
	GL_LoadTextureFromAsset(wall->tex);

	const float halfHeight = wall->height / 2.0f;
	const Vector2 startVertex = v2(wall->a.x, wall->a.y);
	const Vector2 endVertex = v2(wall->b.x, wall->b.y);
	const Vector2 startUV = v2(wall->uvOffset.x, wall->uvOffset.y);
	const Vector2 endUV = v2(wall->uvScale.x * wall->length + wall->uvOffset.x,
							 wall->uvScale.y * wall->height + wall->uvOffset.y);
	const float vertices[8][5] = {
		// X Y Z U V A
		{
			startVertex.x,
			halfHeight,
			startVertex.y,
			startUV.x,
			startUV.y,
		},
		{
			endVertex.x,
			halfHeight,
			endVertex.y,
			endUV.x,
			startUV.y,
		},
		{
			endVertex.x,
			-halfHeight,
			endVertex.y,
			endUV.x,
			endUV.y,
		},
		{
			startVertex.x,
			-halfHeight,
			startVertex.y,
			startUV.x,
			endUV.y,
		},

		// backface
		{
			startVertex.x,
			halfHeight,
			startVertex.y,
			endUV.x,
			startUV.y,
		},
		{
			endVertex.x,
			halfHeight,
			endVertex.y,
			startUV.x,
			startUV.y,
		},
		{
			endVertex.x,
			-halfHeight,
			endVertex.y,
			startUV.x,
			endUV.y,
		},
		{
			startVertex.x,
			-halfHeight,
			startVertex.y,
			endUV.x,
			endUV.y,
		},
	};

	const uint32_t indices[] = {2, 1, 0, 3, 2, 0, 4, 5, 6, 4, 6, 7};

	GL_BindBuffer(glBuffer);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STREAM_DRAW);

	glVertexAttribPointer(actorWallUnshadedVertexLoc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void *)0);
	glEnableVertexAttribArray(actorWallUnshadedVertexLoc);

	glVertexAttribPointer(actorWallUnshadedUvLoc,
						  2,
						  GL_FLOAT,
						  GL_FALSE,
						  5 * sizeof(GLfloat),
						  (void *)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(actorWallUnshadedUvLoc);

	glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, NULL);
}

void GL_RenderMap(const Map *map, const Camera *camera)
{
	GL_Enable3D(); // depth should be clear from frame start

	// glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	// glLineWidth(2);

	mat4 worldViewMatrix;
	GL_GetMatrix(camera, &worldViewMatrix);
	mat4 skyModelWorldMatrix = GLM_MAT4_IDENTITY_INIT;
	glm_translated(skyModelWorldMatrix, VECTOR3_TO_VEC3(camera->transform.position));

	GL_SetMapParams(&worldViewMatrix, map);

	if (map->renderSky)
	{
		GL_RenderModel(LoadModel(MODEL("sky")), skyModelWorldMatrix, 0, 0, COLOR_WHITE);
		GL_ClearDepthOnly(); // prevent sky from clipping into walls
	}

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, lightmap);
	for (size_t i = 0; i < GL_MAX_MAP_MODELS; i++)
	{
		if (mapModels[i] == NULL)
		{
			break;
		}
		if (mapModels[i]->mapModel->material->shader == SHADER_SHADED)
		{
			GL_RenderShadedMapModel(mapModels[i]);
		} else
		{
			GL_RenderUnshadedMapModel(mapModels[i]);
		}
	}
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);

	ListLock(map->actors);
	for (size_t i = 0; i < map->actors.length; i++)
	{
		const Actor *actor = ListGetPointer(map->actors, i);
		if (!actor->wall && !actor->model)
		{
			continue;
		}
		if (!actor->visible)
		{
			continue;
		}

		mat4 actorXfm = GLM_MAT4_IDENTITY_INIT;
		ActorTransformMatrix(actor, &actorXfm);
		if (actor->hasModel)
		{
			GL_RenderModel(actor->model, actorXfm, actor->currentSkinIndex, actor->currentLod, actor->modColor);
		} else
		{
			if (actor->wall == NULL)
			{
				continue;
			}
			if (actor->wall->unshaded)
			{
				GL_DrawUnshadedActorWall(actor, actorXfm);
			} else
			{
				GL_DrawShadedActorWall(actor, actorXfm);
			}
		}
	}
	ListUnlock(map->actors);

	if (map->player.isFreecamActive)
	{
		mat4 playerXfm = GLM_MAT4_IDENTITY_INIT;
		JPH_RMat44 matrix;
		JPH_CharacterVirtual_GetWorldTransform(map->player.joltCharacter, &matrix);
		memcpy(*playerXfm, &matrix, sizeof(mat4));
		GL_RenderModel(LoadModel(MODEL("player")), playerXfm, 0, 0, COLOR_WHITE);
	}

	GL_DrawDebugLines();

	if (map->viewmodel.enabled)
	{
		glClear(GL_DEPTH_BUFFER_BIT);

		mat4 viewModelMatrix;
		GL_GetViewmodelMatrix(map, &viewModelMatrix);

		GL_SharedUniforms uniforms = {
			.fogColor = map->fogColor,
			.fogStart = map->fogStart,
			.fogEnd = map->fogEnd,
			.lightColor = {map->lightColor.r, map->lightColor.g, map->lightColor.b},
			.lightDirection = {0, -(float)PI, 0},
		};
		glm_mat4_copy(viewModelMatrix, uniforms.worldViewMatrix);

		glBindBuffer(GL_UNIFORM_BUFFER, sharedUniformBuffer);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(GL_SharedUniforms), &uniforms, GL_STREAM_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		GL_RenderModel(map->viewmodel.model, GLM_MAT4_IDENTITY, 0, 0, COLOR_WHITE);
	}

	// glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

	GL_Disable3D(map->exposure);
}

void GL_RenderShadedModelPart(const ModelDefinition *model,
							  const mat4 modelWorldMatrix,
							  const uint32_t lod,
							  const size_t materialIndex,
							  Color modColor,
							  const Material *mat)
{
	GL_UseShader(actorShadedShader);
	glActiveTexture(GL_TEXTURE0);
	GL_LoadTextureFromAsset(mat->texture);

	glBindBufferBase(GL_UNIFORM_BUFFER, shadedActorModelSharedUniformsLoc, sharedUniformBuffer);

	glUniformMatrix4fv(shadedActorModelModelWorldMatrixLoc, 1, GL_FALSE,
					   *modelWorldMatrix); // model -> world
	glUniform4fv(shadedActorModelAlbColorLoc, 1, COLOR_TO_ARR(mat->color));
	glUniform4fv(shadedActorModelModColorLoc, 1, COLOR_TO_ARR(modColor));

	GL_LoadModel(model, lod, materialIndex);

	glVertexAttribPointer(shadedActorModelVertexLoc, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(GLfloat), (void *)0);
	glEnableVertexAttribArray(shadedActorModelVertexLoc);

	glVertexAttribPointer(shadedActorModelUvLoc,
						  2,
						  GL_FLOAT,
						  GL_FALSE,
						  12 * sizeof(GLfloat),
						  (void *)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(shadedActorModelUvLoc);

	glVertexAttribPointer(shadedActorModelColorLoc,
						  4,
						  GL_FLOAT,
						  GL_FALSE,
						  12 * sizeof(GLfloat),
						  (void *)(5 * sizeof(GLfloat)));
	glEnableVertexAttribArray(shadedActorModelColorLoc);

	glVertexAttribPointer(shadedActorModelNormalLoc,
						  3,
						  GL_FLOAT,
						  GL_FALSE,
						  12 * sizeof(GLfloat),
						  (void *)(9 * sizeof(GLfloat)));
	glEnableVertexAttribArray(shadedActorModelNormalLoc);

	glDrawElements(GL_TRIANGLES, (int)model->lods[lod]->indexCount[materialIndex], GL_UNSIGNED_INT, NULL);
}

void GL_RenderUnshadedModelPart(const ModelDefinition *model,
								const mat4 modelWorldMatrix,
								const uint32_t lod,
								const size_t materialIndex,
								Color modColor,
								const Material *mat)
{
	GL_UseShader(actorUnshadedShader);
	glActiveTexture(GL_TEXTURE0);
	GL_LoadTextureFromAsset(mat->texture);

	glBindBufferBase(GL_UNIFORM_BUFFER, unshadedActorModelSharedUniformsLoc, sharedUniformBuffer);

	glUniformMatrix4fv(unshadedActorModelModelWorldMatrixLoc, 1, GL_FALSE,
					   *modelWorldMatrix); // model -> world
	glUniform4fv(unshadedActorModelAlbColorLoc, 1, COLOR_TO_ARR(mat->color));
	glUniform4fv(unshadedActorModelModColorLoc, 1, COLOR_TO_ARR(modColor));

	GL_LoadModel(model, lod, materialIndex);

	glVertexAttribPointer(unshadedActorModelVertexLoc, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(GLfloat), (void *)0);
	glEnableVertexAttribArray(unshadedActorModelVertexLoc);

	glVertexAttribPointer(unshadedActorModelUvLoc,
						  2,
						  GL_FLOAT,
						  GL_FALSE,
						  12 * sizeof(GLfloat),
						  (void *)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(unshadedActorModelUvLoc);

	glVertexAttribPointer(unshadedActorModelColorLoc,
						  4,
						  GL_FLOAT,
						  GL_FALSE,
						  12 * sizeof(GLfloat),
						  (void *)(5 * sizeof(GLfloat)));
	glEnableVertexAttribArray(unshadedActorModelColorLoc);

	glDrawElements(GL_TRIANGLES, (int)model->lods[lod]->indexCount[materialIndex], GL_UNSIGNED_INT, NULL);
}

void GL_RenderSkyModelPart(const ModelDefinition *model,
						   const mat4 modelWorldMatrix,
						   const uint32_t lod,
						   const size_t materialIndex)
{
	GL_UseShader(skyShader);

	glActiveTexture(GL_TEXTURE0);
	GL_LoadTextureFromAsset(GetState()->map->skyTexture);

	glBindBufferBase(GL_UNIFORM_BUFFER, skySharedUniformsLoc, sharedUniformBuffer);

	glUniformMatrix4fv(skyModelWorldMatrixLoc, 1, GL_FALSE,
					   *modelWorldMatrix); // model -> world

	GL_LoadModel(model, lod, materialIndex);

	glVertexAttribPointer(skyVertexLoc, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(GLfloat), (void *)0);
	glEnableVertexAttribArray(skyVertexLoc);

	glVertexAttribPointer(skyUvLoc, 2, GL_FLOAT, GL_FALSE, 12 * sizeof(GLfloat), (void *)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(skyUvLoc);

	glVertexAttribPointer(skyColorLoc, 4, GL_FLOAT, GL_FALSE, 12 * sizeof(GLfloat), (void *)(5 * sizeof(GLfloat)));
	glEnableVertexAttribArray(skyColorLoc);

	glDrawElements(GL_TRIANGLES, (int)model->lods[lod]->indexCount[materialIndex], GL_UNSIGNED_INT, NULL);
}

void GL_RenderModel(const ModelDefinition *model,
					const mat4 modelWorldMatrix,
					const uint32_t skin,
					const uint32_t lod,
					const Color modColor)
{
	for (uint32_t material = 0; material < model->materialsPerSkin; material++)
	{
		const uint32_t realSkin = min(skin, model->skinCount - 1);
		const uint32_t *skinIndices = model->skins[realSkin];
		const Material mat = model->materials[skinIndices[material]];
		switch (mat.shader)
		{
			case SHADER_SKY:
				GL_RenderSkyModelPart(model, modelWorldMatrix, lod, material);
				break;
			case SHADER_UNSHADED:
				GL_RenderUnshadedModelPart(model, modelWorldMatrix, lod, material, modColor, &mat);
				break;
			case SHADER_SHADED:
				GL_RenderShadedModelPart(model, modelWorldMatrix, lod, material, modColor, &mat);
				break;
		}
	}
}

void GL_RenderShadedMapModel(const GL_MapModelBuffer *model)
{
	GL_UseShader(mapShadedShader);

	glActiveTexture(GL_TEXTURE0);
	GL_LoadTextureFromAsset(model->mapModel->material->texture);
	glUniform1i(shadedMapModelLightmapLoc, 1);

	const mat4 idty = GLM_MAT4_IDENTITY_INIT;

	glBindBufferBase(GL_UNIFORM_BUFFER, shadedMapModelSharedUniformsLoc, sharedUniformBuffer);

	glUniformMatrix4fv(shadedMapModelModelWorldMatrixLoc, 1, GL_FALSE,
					   *idty); // model -> world

	GL_BindBuffer(model->buffer);

	glVertexAttribPointer(shadedMapModelVertexLoc, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void *)0);
	glEnableVertexAttribArray(shadedMapModelVertexLoc);

	glVertexAttribPointer(shadedMapModelUvLoc,
						  2,
						  GL_FLOAT,
						  GL_FALSE,
						  7 * sizeof(GLfloat),
						  (void *)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(shadedMapModelUvLoc);

	glVertexAttribPointer(shadedMapModelUv2Loc,
						  2,
						  GL_FLOAT,
						  GL_FALSE,
						  7 * sizeof(GLfloat),
						  (void *)(5 * sizeof(GLfloat)));
	glEnableVertexAttribArray(shadedMapModelUv2Loc);

	glDrawElements(GL_TRIANGLES, (int)model->mapModel->indexCount, GL_UNSIGNED_INT, NULL);
}

void GL_RenderUnshadedMapModel(const GL_MapModelBuffer *model)
{
	GL_UseShader(mapUnshadedShader);

	glActiveTexture(GL_TEXTURE0);
	GL_LoadTextureFromAsset(model->mapModel->material->texture);

	const mat4 idty = GLM_MAT4_IDENTITY_INIT;

	glBindBufferBase(GL_UNIFORM_BUFFER, unshadedMapModelSharedUniformsLoc, sharedUniformBuffer);

	glUniformMatrix4fv(unshadedMapModelModelWorldMatrixLoc, 1, GL_FALSE,
					   *idty); // model -> world

	GL_BindBuffer(model->buffer);

	glVertexAttribPointer(unshadedMapModelVertexLoc, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void *)0);
	glEnableVertexAttribArray(unshadedMapModelVertexLoc);

	glVertexAttribPointer(unshadedMapModelUvLoc,
						  2,
						  GL_FLOAT,
						  GL_FALSE,
						  7 * sizeof(GLfloat),
						  (void *)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(unshadedMapModelUvLoc);

	glDrawElements(GL_TRIANGLES, (int)model->mapModel->indexCount, GL_UNSIGNED_INT, NULL);
}

void GL_LoadMap(const Map *map)
{
	GL_DestroyMapModels();
	if (glIsTexture(lightmap))
	{
		glDeleteTextures(1, &lightmap);
	}
	glActiveTexture(GL_TEXTURE1);
	if (map)
	{
		for (size_t i = 0; i < map->modelCount; i++)
		{
			GL_MapModelBuffer *mmb = malloc(sizeof(GL_MapModelBuffer));
			CheckAlloc(mmb);
			mapModels[i] = mmb;
			mmb->mapModel = &map->models[i];
			mmb->buffer = GL_ConstructBuffer();
			GL_BindBuffer(mmb->buffer);
			glBufferData(GL_ARRAY_BUFFER,
						 (GLsizeiptr)(mmb->mapModel->vertexCount * sizeof(MapVertex)),
						 mmb->mapModel->vertices,
						 GL_STREAM_DRAW);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER,
						 (GLsizeiptr)(mmb->mapModel->indexCount * sizeof(uint32_t)),
						 mmb->mapModel->indices,
						 GL_STREAM_DRAW);
		}

		glGenTextures(1, &lightmap);
		glBindTexture(GL_TEXTURE_2D, lightmap);
		glTexImage2D(GL_TEXTURE_2D,
					 0,
					 GL_RGBA16F,
					 (GLsizei)map->lightmapWidth,
					 (GLsizei)map->lightmapHeight,
					 0,
					 GL_RGBA,
					 GL_HALF_FLOAT,
					 map->lightmapPixels);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	glBindTexture(GL_TEXTURE_2D, 0);
}

void GL_SetMapParams(mat4 *modelViewProjection, const Map *map)
{
	GL_SharedUniforms uniforms;
	glm_mat4_copy(*modelViewProjection, uniforms.worldViewMatrix);
	uniforms.fogColor = map->fogColor;
	uniforms.fogStart = (float)map->fogStart;
	uniforms.fogEnd = (float)map->fogEnd;
	uniforms.lightColor[0] = map->lightColor.r;
	uniforms.lightColor[1] = map->lightColor.g;
	uniforms.lightColor[2] = map->lightColor.b;

	uniforms.lightDirection[0] = -cosf(map->lightAngle.x) * sinf(map->lightAngle.y);
	uniforms.lightDirection[1] = sinf(map->lightAngle.x);
	uniforms.lightDirection[2] = -cosf(map->lightAngle.x) * cosf(map->lightAngle.y);

	glBindBuffer(GL_UNIFORM_BUFFER, sharedUniformBuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(GL_SharedUniforms), &uniforms, GL_STREAM_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

inline void GL_Enable3D(void)
{
	GL_Begin3DPass();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
}

inline void GL_Disable3D(const float exposure)
{
	GL_End3DPass(exposure);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_MULTISAMPLE);
}

void GL_GetMatrix(const Camera *camera, mat4 *modelViewProjectionMatrix)
{
	mat4 perspectiveMatrix;
	glm_perspective(glm_rad(camera->fov),
					ScaledWindowWidthFloat() / ScaledWindowHeightFloat(),
					NEAR_Z,
					FAR_Z,
					perspectiveMatrix);

	versor rotationQuat;
	QUAT_TO_VERSOR(camera->transform.rotation, rotationQuat);

	mat4 viewMatrix;
	glm_quat_look(VECTOR3_TO_VEC3(camera->transform.position), rotationQuat, viewMatrix);

	glm_mat4_mul(perspectiveMatrix, viewMatrix, *modelViewProjectionMatrix);
}

void GL_GetViewmodelMatrix(const Map *map, mat4 *out)
{
	mat4 perspectiveMatrix;
	glm_perspective(glm_rad(VIEWMODEL_FOV),
					ScaledWindowWidthFloat() / ScaledWindowHeightFloat(),
					VIEWMODEL_NEAR_Z,
					VIEWMODEL_FAR_Z,
					perspectiveMatrix);

	mat4 translationMatrix = GLM_MAT4_IDENTITY_INIT;
	glm_translate(translationMatrix, VECTOR3_TO_VEC3(map->viewmodel.transform.position));

	mat4 rotationMatrix = GLM_MAT4_IDENTITY_INIT;
	// TODO rotation other than yaw
	glm_rotate(rotationMatrix, JPH_Quat_GetRotationAngle(&map->viewmodel.transform.rotation, &Vector3_AxisY), GLM_YUP);

	glm_mat4_mul(translationMatrix, rotationMatrix, translationMatrix);
	glm_mat4_mul(perspectiveMatrix, translationMatrix, *out);
}
