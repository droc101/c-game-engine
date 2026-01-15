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
#include <engine/subsystem/Logging.h>
#include <joltc/Math/Quat.h>
#include <joltc/Math/Vector3.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void GL_DrawShadedActorWall(const Actor *actor, const mat4 actorXfm)
{
	const ActorWall *wall = actor->actorWall;

	glUseProgram(shadedWallShader->program);

	glBindBufferBase(GL_UNIFORM_BUFFER, shadedWallSharedUniformsLoc, sharedUniformBuffer);

	glUniformMatrix4fv(shadedWallTransformMatrixLoc, 1, GL_FALSE, *actorXfm);

	GL_LoadTextureFromAsset(wall->tex);

	const float halfHeight = wall->height / 2.0f;
	const Vector2 startVertex = v2(wall->a.x, wall->a.y);
	const Vector2 endVertex = v2(wall->b.x, wall->b.y);
	const Vector2 startUV = v2(wall->uvOffset, 0);
	const Vector2 endUV = v2(wall->uvScale * wall->length + wall->uvOffset, 1);
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

	glBindVertexArray(glBuffer->vertexArrayObject);

	glBindBuffer(GL_ARRAY_BUFFER, glBuffer->vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glBuffer->elementBufferObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STREAM_DRAW);

	const GLint posAttrLoc = glGetAttribLocation(shadedWallShader->program, "VERTEX");
	glVertexAttribPointer(posAttrLoc, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void *)0);
	glEnableVertexAttribArray(posAttrLoc);

	const GLint texAttrLoc = glGetAttribLocation(shadedWallShader->program, "VERTEX_UV");
	glVertexAttribPointer(texAttrLoc, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void *)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(texAttrLoc);

	const GLint angleAttrLoc = glGetAttribLocation(shadedWallShader->program, "VERTEX_ANGLE");
	glVertexAttribPointer(angleAttrLoc, 1, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void *)(5 * sizeof(GLfloat)));
	glEnableVertexAttribArray(angleAttrLoc);

	glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, NULL);
}

void GL_DrawUnshadedActorWall(const Actor *actor, const mat4 actorXfm)
{
	const ActorWall *wall = actor->actorWall;

	glUseProgram(unshadedWallShader->program);

	glBindBufferBase(GL_UNIFORM_BUFFER, unshadedWallSharedUniformsLoc, sharedUniformBuffer);

	glUniformMatrix4fv(unshadedWallTransformMatrixLoc, 1, GL_FALSE, *actorXfm);

	GL_LoadTextureFromAsset(wall->tex);

	const float halfHeight = wall->height / 2.0f;
	const Vector2 startVertex = v2(wall->a.x, wall->a.y);
	const Vector2 endVertex = v2(wall->b.x, wall->b.y);
	const Vector2 startUV = v2(wall->uvOffset, 0);
	const Vector2 endUV = v2(wall->uvScale * wall->length + wall->uvOffset, 1);
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

	glBindVertexArray(glBuffer->vertexArrayObject);

	glBindBuffer(GL_ARRAY_BUFFER, glBuffer->vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glBuffer->elementBufferObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STREAM_DRAW);

	const GLint posAttrLoc = glGetAttribLocation(unshadedWallShader->program, "VERTEX");
	glVertexAttribPointer(posAttrLoc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void *)0);
	glEnableVertexAttribArray(posAttrLoc);

	const GLint texAttrLoc = glGetAttribLocation(unshadedWallShader->program, "VERTEX_UV");
	glVertexAttribPointer(texAttrLoc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void *)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(texAttrLoc);

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

	GL_RenderModel(LoadModel(MODEL("sky")), skyModelWorldMatrix, 0, 0, COLOR_WHITE);
	GL_ClearDepthOnly(); // prevent sky from clipping into walls

	for (size_t i = 0; i < GL_MAX_MAP_MODELS; i++)
	{
		if (mapModels[i] == NULL)
		{
			break;
		}
		GL_RenderMapModel(mapModels[i]);
	}

	ListLock(map->actors);
	for (size_t i = 0; i < map->actors.length; i++)
	{
		const Actor *actor = ListGetPointer(map->actors, i);
		if (!actor->actorWall && !actor->actorModel)
		{
			continue;
		}
		if (!actor->visible)
		{
			continue;
		}

		mat4 actorXfm = GLM_MAT4_IDENTITY_INIT;
		ActorTransformMatrix(actor, &actorXfm);
		if (actor->actorModel == NULL)
		{
			if (actor->actorWall == NULL)
			{
				continue;
			}
			if (actor->actorWall->unshaded)
			{
				GL_DrawUnshadedActorWall(actor, actorXfm);
			} else
			{
				GL_DrawShadedActorWall(actor, actorXfm);
			}
		} else
		{
			GL_RenderModel(actor->actorModel, actorXfm, actor->currentSkinIndex, actor->currentLod, actor->modColor);
		}
	}
	ListUnlock(map->actors);

#ifdef THIRDPERSON
	mat4 playerXfm = GLM_MAT4_IDENTITY_INIT;
	JPH_RMat44 matrix;
	JPH_CharacterVirtual_GetWorldTransform(map->player.joltCharacter, &matrix);
	memcpy(*playerXfm, &matrix, sizeof(mat4));
	GL_RenderModel(LoadModel(MODEL("player")), playerXfm, 0, 0, COLOR_WHITE);
#endif

	GL_DrawDebugLines();

	if (GetState()->viewmodel.enabled)
	{
		glClear(GL_DEPTH_BUFFER_BIT);

		mat4 viewModelMatrix;
		GL_GetViewmodelMatrix(&viewModelMatrix);

		GL_SharedUniforms uniforms = {
			.fogStart = 1000,
			.fogEnd = 1001,
			.lightColor = {map->lightColor.r, map->lightColor.g, map->lightColor.b},
			.lightDirection = {0, -(float)PI, 0},
		};
		glm_mat4_copy(viewModelMatrix, uniforms.worldViewMatrix);

		glBindBuffer(GL_UNIFORM_BUFFER, sharedUniformBuffer);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(GL_SharedUniforms), &uniforms, GL_STREAM_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		GL_RenderModel(GetState()->viewmodel.model, GLM_MAT4_IDENTITY, 0, 0, COLOR_WHITE);
	}

	// glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

	GL_Disable3D();
}


void GL_RenderModelPart(const ModelDefinition *model,
						const mat4 modelWorldMatrix,
						const uint32_t lod,
						const size_t material,
						const uint32_t skin,
						Color modColor)
{
	const uint32_t realSkin = min(skin, model->skinCount - 1);
	const uint32_t *skinIndices = model->skins[realSkin];
	const Material mat = model->materials[skinIndices[material]];

	const ModelShader shader = mat.shader;

	const GL_Shader *glShader = NULL;
	switch (shader)
	{
		case SHADER_SKY:
			glShader = skyShader;
			break;
		case SHADER_SHADED:
			glShader = modelShadedShader;
			break;
		case SHADER_UNSHADED:
			glShader = modelUnshadedShader;
			break;
		default:
			LogError("Invalid shader for model drawing\n");
			return;
	}

	glUseProgram(glShader->program);

	if (shader == SHADER_SKY)
	{
		GL_LoadTextureFromAsset(GetState()->map->skyTexture);
	} else
	{
		GL_LoadTextureFromAsset(mat.texture);
	}


	glBindBufferBase(GL_UNIFORM_BUFFER,
					 glGetUniformBlockIndex(glShader->program, "SharedUniforms"),
					 sharedUniformBuffer);

	glUniformMatrix4fv(glGetUniformLocation(glShader->program, "MODEL_WORLD_MATRIX"),
					   1,
					   GL_FALSE,
					   *modelWorldMatrix); // model -> world

	GL_LoadModel(model, lod, material);

	const GLint posAttrLoc = glGetAttribLocation(glShader->program, "VERTEX");
	glVertexAttribPointer(posAttrLoc, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(GLfloat), (void *)0);
	glEnableVertexAttribArray(posAttrLoc);

	const GLint texAttrLoc = glGetAttribLocation(glShader->program, "VERTEX_UV");
	glVertexAttribPointer(texAttrLoc, 2, GL_FLOAT, GL_FALSE, 12 * sizeof(GLfloat), (void *)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(texAttrLoc);

	const GLint colAttrLoc = glGetAttribLocation(glShader->program, "VERTEX_COLOR");
	glVertexAttribPointer(colAttrLoc, 4, GL_FLOAT, GL_FALSE, 12 * sizeof(GLfloat), (void *)(5 * sizeof(GLfloat)));
	glEnableVertexAttribArray(colAttrLoc);

	if (shader == SHADER_SHADED) // other shaders do not take normals
	{
		const GLint normAttrLoc = glGetAttribLocation(glShader->program, "VERTEX_NORMAL");
		glVertexAttribPointer(normAttrLoc, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(GLfloat), (void *)(9 * sizeof(GLfloat)));
		glEnableVertexAttribArray(normAttrLoc);
	}

	if (shader != SHADER_SKY)
	{
		const GLint colUniformLocation = glGetUniformLocation(glShader->program, "albColor");
		glUniform4fv(colUniformLocation, 1, COLOR_TO_ARR(mat.color));

		const GLint modColUniformLocation = glGetUniformLocation(glShader->program, "modColor");
		glUniform4fv(modColUniformLocation, 1, COLOR_TO_ARR(modColor));
	}

	glDrawElements(GL_TRIANGLES, (int)model->lods[lod]->indexCount[material], GL_UNSIGNED_INT, NULL);
}

void GL_RenderModel(const ModelDefinition *model,
					const mat4 modelWorldMatrix,
					const uint32_t skin,
					const uint32_t lod,
					const Color modColor)
{
	for (uint32_t material = 0; material < model->materialsPerSkin; material++)
	{
		GL_RenderModelPart(model, modelWorldMatrix, lod, material, skin, modColor);
	}
}

void GL_RenderMapModel(const GL_MapModelBuffer *model)
{
	const ModelShader shader = model->mapModel->material->shader;

	const GL_Shader *glShader = NULL;
	switch (shader)
	{
		case SHADER_SHADED:
			glShader = modelShadedShader;
			break;
		case SHADER_UNSHADED:
			glShader = modelUnshadedShader;
			break;
		default:
			LogError("Invalid shader for model drawing\n");
			return;
	}

	glUseProgram(glShader->program);

	GL_LoadTextureFromAsset(model->mapModel->material->texture);

	mat4 idty = GLM_MAT4_IDENTITY_INIT;

	glBindBufferBase(GL_UNIFORM_BUFFER,
					 glGetUniformBlockIndex(glShader->program, "SharedUniforms"),
					 sharedUniformBuffer);

	glUniformMatrix4fv(glGetUniformLocation(glShader->program, "MODEL_WORLD_MATRIX"),
					   1,
					   GL_FALSE,
					   *idty); // model -> world

	glBindVertexArray(model->buffer->vertexArrayObject);

	glBindBuffer(GL_ARRAY_BUFFER, model->buffer->vertexBufferObject);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->buffer->elementBufferObject);

	const GLint posAttrLoc = glGetAttribLocation(glShader->program, "VERTEX");
	glVertexAttribPointer(posAttrLoc, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(GLfloat), (void *)0);
	glEnableVertexAttribArray(posAttrLoc);

	const GLint texAttrLoc = glGetAttribLocation(glShader->program, "VERTEX_UV");
	glVertexAttribPointer(texAttrLoc, 2, GL_FLOAT, GL_FALSE, 12 * sizeof(GLfloat), (void *)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(texAttrLoc);

	const GLint colAttrLoc = glGetAttribLocation(glShader->program, "VERTEX_COLOR");
	glVertexAttribPointer(colAttrLoc, 4, GL_FLOAT, GL_FALSE, 12 * sizeof(GLfloat), (void *)(5 * sizeof(GLfloat)));
	glEnableVertexAttribArray(colAttrLoc);

	if (shader == SHADER_SHADED) // other shaders do not take normals
	{
		const GLint normAttrLoc = glGetAttribLocation(glShader->program, "VERTEX_NORMAL");
		glVertexAttribPointer(normAttrLoc, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(GLfloat), (void *)(9 * sizeof(GLfloat)));
		glEnableVertexAttribArray(normAttrLoc);
	}

	const GLint colUniformLocation = glGetUniformLocation(glShader->program, "albColor");
	glUniform4fv(colUniformLocation, 1, COLOR_TO_ARR(COLOR_WHITE));

	const GLint modColUniformLocation = glGetUniformLocation(glShader->program, "modColor");
	glUniform4fv(modColUniformLocation, 1, COLOR_TO_ARR(COLOR_WHITE));

	glDrawElements(GL_TRIANGLES, (int)model->mapModel->indexCount, GL_UNSIGNED_INT, NULL);
}

void GL_LoadMap(const Map *map)
{
	GL_DestroyMapModels();

	for (size_t i = 0; i < map->modelCount; i++)
	{
		GL_MapModelBuffer *mmb = malloc(sizeof(GL_MapModelBuffer));
		CheckAlloc(mmb);
		mapModels[i] = mmb;
		mmb->mapModel = &map->models[i];
		mmb->buffer = GL_ConstructBuffer();

		glBindVertexArray(mmb->buffer->vertexArrayObject);

		glBindBuffer(GL_ARRAY_BUFFER, mmb->buffer->vertexBufferObject);
		glBufferData(GL_ARRAY_BUFFER,
					 (GLsizeiptr)(mmb->mapModel->vertexCount * sizeof(MapVertex)),
					 mmb->mapModel->vertices,
					 GL_STREAM_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mmb->buffer->elementBufferObject);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,
					 (GLsizeiptr)(mmb->mapModel->indexCount * sizeof(uint32_t)),
					 mmb->mapModel->indices,
					 GL_STREAM_DRAW);
	}
}

void GL_SetMapParams(mat4 *modelViewProjection, const Map *map)
{
	GL_SharedUniforms uniforms;
	glm_mat4_copy(*modelViewProjection, uniforms.worldViewMatrix);
	uniforms.fogColor = COLOR(map->fogColor);
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
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
}

inline void GL_Disable3D()
{
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
#ifdef THIRDPERSON
	vec3 pos;
	glm_quat_rotatev(rotationQuat, GLM_ZUP, pos);
	glm_vec3_add(VECTOR3_TO_VEC3(camera->transform.position), pos, pos);
	glm_quat_look(pos, rotationQuat, viewMatrix);
#else
	glm_quat_look(VECTOR3_TO_VEC3(camera->transform.position), rotationQuat, viewMatrix);
#endif

	glm_mat4_mul(perspectiveMatrix, viewMatrix, *modelViewProjectionMatrix);
}

void GL_GetViewmodelMatrix(mat4 *out)
{
	mat4 perspectiveMatrix;
	glm_perspective(glm_rad(VIEWMODEL_FOV),
					ScaledWindowWidthFloat() / ScaledWindowHeightFloat(),
					NEAR_Z,
					FAR_Z,
					perspectiveMatrix);

	mat4 translationMatrix = GLM_MAT4_IDENTITY_INIT;
	glm_translate(translationMatrix, VECTOR3_TO_VEC3(GetState()->viewmodel.transform.position));

	mat4 rotationMatrix = GLM_MAT4_IDENTITY_INIT;
	// TODO rotation other than yaw
	glm_rotate(rotationMatrix,
			   JPH_Quat_GetRotationAngle(&GetState()->viewmodel.transform.rotation, &Vector3_AxisY),
			   GLM_YUP);

	glm_mat4_mul(translationMatrix, rotationMatrix, translationMatrix);
	glm_mat4_mul(perspectiveMatrix, translationMatrix, *out);
}
