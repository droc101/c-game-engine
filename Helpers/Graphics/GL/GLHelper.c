//
// Created by droc101 on 9/30/2024.
//

#include "GLHelper.h"
#include <cglm/cglm.h>
#include <joltc/Math/Quat.h>
#include <joltc/Math/Vector3.h>
#include <SDL_error.h>
#include <SDL_video.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../../Structs/Actor.h"
#include "../../../Structs/Color.h"
#include "../../../Structs/GlobalState.h"
#include "../../../Structs/Level.h"
#include "../../../Structs/Options.h"
#include "../../../Structs/Vector2.h"
#include "../../../Structs/Wall.h"
#include "../../Core/AssetLoaders/ModelLoader.h"
#include "../../Core/AssetLoaders/ShaderLoader.h"
#include "../../Core/AssetLoaders/TextureLoader.h"
#include "../../Core/AssetReader.h"
#include "../../Core/Error.h"
#include "../../Core/List.h"
#include "../../Core/Logging.h"
#include "../../Core/MathEx.h"
#include "../../Core/Physics/Physics.h"
#include "../RenderingHelpers.h"
#include "GLInternal.h"

SDL_GLContext ctx;

GL_Shader *uiTexturedShader;
GL_Shader *uiColoredShader;
GL_Shader *wallShader;
GL_Shader *floorAndCeilingShader;
GL_Shader *skyShader;
GL_Shader *modelUnshadedShader;
GL_Shader *modelShadedShader;

GL_Buffer *glBuffer;

GLuint glTextures[MAX_TEXTURES];
int glNextFreeSlot = 0;
int glAssetTextureMap[MAX_TEXTURES];
char glLastError[512];

GL_ModelBuffers *glModels[MAX_MODELS];
GL_WallBuffers *glWalls[GL_MAX_WALL_BUFFERS];

GLuint sharedUniformBuffer;

#pragma region Shader Variable Locations

GLint floorTextureLoc;
GLint floorShadeLoc;
GLint floorHeightLoc;
GLint floorSharedUniformsLoc;

GLint hudColoredColorLoc; // TODO: confusing name -- location of the color uniform in the colored shader

GLint hudTexturedTextureLoc; // TODO: confusing name -- location of the texture uniform in the textured shader
GLint hudTexturedColorLoc;
GLint hudTexturedRegionLoc;

GLint wallTextureLoc;
GLint wallModelWorldMatrixLoc;
GLint wallSharedUniformsLoc;
GLint wallTransformMatrixLoc;

#pragma endregion


#pragma region Init/Destroy

void LoadShaderLocations()
{
	floorTextureLoc = glGetUniformLocation(floorAndCeilingShader->program, "alb");
	floorShadeLoc = glGetUniformLocation(floorAndCeilingShader->program, "shade");
	floorHeightLoc = glGetUniformLocation(floorAndCeilingShader->program, "height");
	floorSharedUniformsLoc = glGetUniformBlockIndex(floorAndCeilingShader->program, "SharedUniforms");

	hudColoredColorLoc = glGetUniformLocation(uiColoredShader->program, "col");

	hudTexturedTextureLoc = glGetUniformLocation(uiTexturedShader->program, "alb");
	hudTexturedColorLoc = glGetUniformLocation(uiTexturedShader->program, "col");
	hudTexturedRegionLoc = glGetUniformLocation(uiTexturedShader->program, "region");

	wallTextureLoc = glGetUniformLocation(wallShader->program, "alb");
	wallModelWorldMatrixLoc = glGetUniformLocation(wallShader->program, "MODEL_WORLD_MATRIX");
	wallSharedUniformsLoc = glGetUniformBlockIndex(wallShader->program, "SharedUniforms");
	wallTransformMatrixLoc = glGetUniformLocation(wallShader->program, "transformMatrix");
}

bool GL_PreInit()
{
	LogDebug("Pre-initializing OpenGL...\n");
	const bool msaaEnabled = GetState()->options.msaa != MSAA_NONE;
	if (SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, msaaEnabled) != 0)
	{
		LogError("Failed to set MSAA buffers attribute: %s\n", SDL_GetError());
	}
	if (msaaEnabled)
	{
		int mssaValue = 0;
		switch (GetState()->options.msaa)
		{
			case MSAA_2X:
				mssaValue = 2;
				break;
			case MSAA_4X:
				mssaValue = 4;
				break;
			case MSAA_8X:
				mssaValue = 8;
				break;
			default:
				GL_Error("Invalid MSAA value!");
				return false;
		}
		if (SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, mssaValue) != 0)
		{
			LogError("Failed to set MSAA samples attribute: %s\n", SDL_GetError());
		}
	}
	TestSDLFunction(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, GL_VERSION_MAJOR),
					"Failed to set OpenGL major version",
					GL_INIT_FAIL_MSG);
	TestSDLFunction(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, GL_VERSION_MINOR),
					"Failed to set OpenGL minor version",
					GL_INIT_FAIL_MSG);
	TestSDLFunction(SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1),
					"Failed to set OpenGL accelerated visual",
					GL_INIT_FAIL_MSG);
	TestSDLFunction(SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, GL_PROFILE),
					"Failed to set OpenGL profile",
					GL_INIT_FAIL_MSG);
	TestSDLFunction(SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1),
					"Failed to set OpenGL double buffer",
					GL_INIT_FAIL_MSG);
	TestSDLFunction(SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8), "Failed to set OpenGL red-size", "Failed to start OpenGL");
	TestSDLFunction(SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8), "Failed to set OpenGL green-size", GL_INIT_FAIL_MSG);
	TestSDLFunction(SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8), "Failed to set OpenGL blue-size", GL_INIT_FAIL_MSG);
	TestSDLFunction(SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8), "Failed to set OpenGL alpha-size", GL_INIT_FAIL_MSG);
	TestSDLFunction(SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24),
					"Failed to set OpenGL depth buffer size",
					GL_INIT_FAIL_MSG);

	memset(glAssetTextureMap, -1, MAX_TEXTURES * sizeof(int));
	memset(glTextures, 0, sizeof(glTextures));

	return true;
}

bool GL_Init(SDL_Window *wnd)
{
	LogDebug("Initializing OpenGL renderer...\n");

	ctx = SDL_GL_CreateContext(wnd);
	if (ctx == NULL)
	{
		LogError("SDL_GL_CreateContext Error: %s\n", SDL_GetError());
		GL_Error("Failed to create OpenGL context");
		return false;
	}

	TestSDLFunction_NonFatal(SDL_GL_SetSwapInterval(GetState()->options.vsync ? 1 : 0), "Failed to set VSync");

	glewExperimental = GL_TRUE; // Please expose OpenGL 3.x+ interfaces
	const GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		SDL_GL_DeleteContext(ctx);
		GL_Error(GL_INIT_FAIL_MSG);
		return false;
	}

	// Ensure we have GL 3.3 or higher
	if (!GL_VERSION_CHECK)
	{
		SDL_GL_DeleteContext(ctx);
		GL_Error(GL_INIT_FAIL_MSG);
		return false;
	}


#ifdef BUILDSTYLE_DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(GL_DebugMessageCallback, NULL);
#endif

	uiTexturedShader = GL_ConstructShaderFromAssets(SHADER("gl/hud_textured_f"), SHADER("gl/hud_textured_v"));
	uiColoredShader = GL_ConstructShaderFromAssets(SHADER("gl/hud_color_f"), SHADER("gl/hud_color_v"));
	wallShader = GL_ConstructShaderFromAssets(SHADER("gl/wall_f"), SHADER("gl/wall_v"));
	floorAndCeilingShader = GL_ConstructShaderFromAssets(SHADER("gl/floor_f"), SHADER("gl/floor_v"));
	skyShader = GL_ConstructShaderFromAssets(SHADER("gl/sky_f"), SHADER("gl/sky_v"));
	modelShadedShader = GL_ConstructShaderFromAssets(SHADER("gl/model_shaded_f"), SHADER("gl/model_shaded_v"));
	modelUnshadedShader = GL_ConstructShaderFromAssets(SHADER("gl/model_unshaded_f"), SHADER("gl/model_unshaded_v"));

	if (!uiTexturedShader ||
		!uiColoredShader ||
		!wallShader ||
		!floorAndCeilingShader ||
		!skyShader ||
		!modelShadedShader ||
		!modelUnshadedShader)
	{
		GL_Error("Failed to compile shaders");
		return false;
	}

	LoadShaderLocations();

	glBuffer = GL_ConstructBuffer();

	glGenBuffers(1, &sharedUniformBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, sharedUniformBuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(GL_SharedUniforms), NULL, GL_STREAM_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glClearColor(0, 0, 0, 1);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glDisable(GL_SCISSOR_TEST);

	char *vendor = (char *)glGetString(GL_VENDOR);
	char *renderer = (char *)glGetString(GL_RENDERER);
	char *version = (char *)glGetString(GL_VERSION);
	char *shadingLanguage = (char *)glGetString(GL_SHADING_LANGUAGE_VERSION);

	LogInfo("OpenGL Initialized\n");
	LogInfo("OpenGL Vendor: %s\n", vendor);
	LogInfo("OpenGL Renderer: %s\n", renderer);
	LogInfo("OpenGL Version: %s\n", version);
	LogInfo("GLSL: %s\n", shadingLanguage);

	fflush(stdout);

	GL_Disable3D();

	return true;
}

void GL_DestroyGL()
{
	LogDebug("Cleaning up OpenGL renderer...\n");
	GL_DestroyShader(uiTexturedShader);
	GL_DestroyShader(uiColoredShader);
	GL_DestroyShader(wallShader);
	GL_DestroyShader(floorAndCeilingShader);
	GL_DestroyShader(skyShader);
	GL_DestroyShader(modelShadedShader);
	GL_DestroyShader(modelUnshadedShader);
	glUseProgram(0);
	glDisableVertexAttribArray(0);
	GL_DestroyBuffer(glBuffer);
	glDeleteBuffers(1, &sharedUniformBuffer);
	GL_DestroyWallBuffers();
	for (int i = 0; i < MAX_TEXTURES; i++)
	{
		if (glTextures[i] != 0)
		{
			glDeleteTextures(1, &glTextures[i]);
		}
	}
	for (int i = 0; i < MAX_MODELS; i++)
	{
		if (glModels[i] != NULL)
		{
			for (uint32_t j = 0; j < glModels[i]->lodCount; j++)
			{
				GL_DestroyBuffer(glModels[i]->buffers[j]);
			}
			free(glModels[i]->buffers);
			free(glModels[i]);
		}
	}
	SDL_GL_DeleteContext(ctx);
}

#pragma endregion


#pragma region GL Objects

GL_Shader *GL_ConstructShaderFromAssets(const char *fsh, const char *vsh)
{
	Shader *fragmentSource = LoadShader(fsh);
	Shader *vertexSource = LoadShader(vsh);
	if (fragmentSource == NULL || vertexSource == NULL)
	{
		Error("Failed to load shaders!");
	}
	GL_Shader *shd = GL_ConstructShader(fragmentSource->glsl, vertexSource->glsl);
	FreeShader(fragmentSource);
	FreeShader(vertexSource);
	return shd;
}

GL_Shader *GL_ConstructShader(const char *fsh, const char *vsh)
{
	GLint status = 0;
	char errorBuffer[512];

	GL_Shader *shader = malloc(sizeof(GL_Shader));
	CheckAlloc(shader);

	shader->vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(shader->vertexShader, 1, (const GLchar *const *)&vsh, NULL);
	glCompileShader(shader->vertexShader);
	glGetShaderiv(shader->vertexShader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE)
	{
		glGetShaderInfoLog(shader->vertexShader, sizeof(errorBuffer), NULL, errorBuffer);
		errorBuffer[sizeof(errorBuffer) - 1] = '\0';
		Error(errorBuffer);
	}

	shader->fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(shader->fragmentShader, 1, (const GLchar *const *)&fsh, NULL);
	glCompileShader(shader->fragmentShader);
	glGetShaderiv(shader->fragmentShader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE)
	{
		glGetShaderInfoLog(shader->fragmentShader, sizeof(errorBuffer), NULL, errorBuffer);
		errorBuffer[sizeof(errorBuffer) - 1] = '\0';
		LogError(errorBuffer);
		free(shader);
		return NULL;
	}

	shader->program = glCreateProgram();
	glAttachShader(shader->program, shader->vertexShader);
	glAttachShader(shader->program, shader->fragmentShader);
	glBindFragDataLocation(shader->program, 0, "COLOR");
	glLinkProgram(shader->program);

	glGetProgramiv(shader->program, GL_LINK_STATUS, &status);
	if (status != GL_TRUE)
	{
		glGetProgramInfoLog(shader->program, sizeof(errorBuffer), NULL, errorBuffer);
		errorBuffer[sizeof(errorBuffer) - 1] = '\0';
		LogError(errorBuffer);
		free(shader);
		return NULL;
	}

	return shader;
}

void GL_DestroyShader(GL_Shader *shd)
{
	glDeleteShader(shd->vertexShader);
	glDeleteShader(shd->fragmentShader);
	glDeleteProgram(shd->program);
	free(shd);
	shd = NULL;
}

GL_Buffer *GL_ConstructBuffer()
{
	GL_Buffer *buffer = malloc(sizeof(GL_Buffer));
	CheckAlloc(buffer);

	glGenVertexArrays(1, &buffer->vertexArrayObject);
	glGenBuffers(1, &buffer->vertexBufferObject);
	glGenBuffers(1, &buffer->elementBufferObject);

	return buffer;
}

void GL_DestroyBuffer(GL_Buffer *buffer)
{
	glDeleteVertexArrays(1, &buffer->vertexArrayObject);
	glDeleteBuffers(1, &buffer->vertexBufferObject);
	glDeleteBuffers(1, &buffer->elementBufferObject);
	free(buffer);
}

void GL_LoadTextureFromAsset(const char *texture)
{
	const Image *image = LoadImage(texture);

	// if the texture is already loaded, don't load it again
	if (glAssetTextureMap[image->id] != -1)
	{
		if (glIsTexture(glTextures[glAssetTextureMap[image->id]]))
		{
			glBindTexture(GL_TEXTURE_2D, glTextures[glAssetTextureMap[image->id]]);
			return;
		}
	}

	const int slot = GL_RegisterTexture(image);

	glAssetTextureMap[image->id] = slot;
}

int GL_RegisterTexture(const Image *image)
{
	const int slot = glNextFreeSlot;

	glGenTextures(1, &glTextures[slot]);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, glTextures[slot]);
	glTexImage2D(GL_TEXTURE_2D,
				 0,
				 GL_RGBA,
				 (GLsizei)image->width,
				 (GLsizei)image->height,
				 0,
				 GL_RGBA,
				 GL_UNSIGNED_BYTE,
				 image->pixelData);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, -1.5f);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, image->repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, image->repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);

	if (GetState()->options.mipmaps && image->mipmaps)
	{
		glGenerateMipmap(GL_TEXTURE_2D);
		if (image->filter)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		} else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}
	} else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, image->filter ? GL_LINEAR : GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, image->filter ? GL_LINEAR : GL_NEAREST);
	}

	glTexSubImage2D(GL_TEXTURE_2D,
					0,
					0,
					0,
					(GLsizei)image->width,
					(GLsizei)image->height,
					GL_RGBA,
					GL_UNSIGNED_INT_8_8_8_8_REV,
					image->pixelData);

	glNextFreeSlot++;

	return slot;
}

void GL_LoadModel(const ModelDefinition *model, const uint32_t lod, const size_t material)
{
	if (glModels[model->id] != NULL)
	{
		const GL_ModelBuffers *modelBuffer = glModels[model->id];
		const GL_Buffer *lodBuffer = modelBuffer->buffers[lod];
		const GL_Buffer materialBuffer = lodBuffer[material];
		glBindVertexArray(materialBuffer.vertexArrayObject);
		glBindBuffer(GL_ARRAY_BUFFER, materialBuffer.vertexBufferObject);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, materialBuffer.elementBufferObject);
		return;
	}
	GL_ModelBuffers *buf = malloc(sizeof(GL_ModelBuffers));
	CheckAlloc(buf);
	buf->lodCount = model->lodCount;
	buf->materialCount = model->materialsPerSkin;
	buf->buffers = malloc(sizeof(GL_Buffer *) * model->lodCount);
	CheckAlloc(buf->buffers);

	for (uint32_t l = 0; l < buf->lodCount; l++)
	{
		buf->buffers[l] = malloc(sizeof(GL_Buffer) * model->materialsPerSkin);
		CheckAlloc(buf->buffers[l]);

		for (uint32_t m = 0; m < buf->materialCount; m++)
		{
			GL_Buffer *modelBuffer = &buf->buffers[l][m];
			glGenVertexArrays(1, &modelBuffer->vertexArrayObject);
			glGenBuffers(1, &modelBuffer->vertexBufferObject);
			glGenBuffers(1, &modelBuffer->elementBufferObject);

			glBindVertexArray(modelBuffer->vertexArrayObject);

			glBindBuffer(GL_ARRAY_BUFFER, modelBuffer->vertexBufferObject);
			glBufferData(GL_ARRAY_BUFFER,
						 (long)(model->lods[l]->vertexCount * sizeof(float) * 12),
						 model->lods[l]->vertexData,
						 GL_STATIC_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelBuffer->elementBufferObject);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER,
						 (long)(model->lods[l]->indexCount[m] * sizeof(uint32_t)),
						 model->lods[l]->indexData[m],
						 GL_STATIC_DRAW);
		}
	}

	glModels[model->id] = buf;
}

void GL_DestroyWallBuffers()
{
	size_t i = 0;
	while (glWalls[i] != NULL && i < GL_MAX_WALL_BUFFERS)
	{
		GL_WallBuffers *wallBuffer = glWalls[i];
		GL_DestroyBuffer(wallBuffer->buffer);
		free(wallBuffer);
		glWalls[i] = NULL;
		i++;
	}
}

GL_WallBuffers *GL_GetWallBuffer(const char *texture)
{
	size_t i = 0;
	while (glWalls[i] != NULL &&
		   i < GL_MAX_WALL_BUFFERS) // TODO: maybe look into a faster lookup, but also this is level load sooooooo
	{
		if (strcmp(glWalls[i]->texture, texture) == 0)
		{
			return glWalls[i];
		}
		i++;
	}
	if (i >= GL_MAX_WALL_BUFFERS)
	{
		Error("Too many wall buffers allocated, increase GL_MAX_WALL_BUFFERS");
	}
	GL_WallBuffers *newBuffer = malloc(sizeof(GL_WallBuffers));
	CheckAlloc(newBuffer);
	newBuffer->buffer = GL_ConstructBuffer();
	glBindBuffer(GL_ARRAY_BUFFER, newBuffer->buffer->vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 24 * GL_MAX_WALLS_PER_BUFFER, NULL, GL_STREAM_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, newBuffer->buffer->elementBufferObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * 6 * GL_MAX_WALLS_PER_BUFFER, NULL, GL_STREAM_DRAW);
	strncpy(newBuffer->texture, texture, sizeof(newBuffer->texture) - 1);
	newBuffer->wallCount = 0;
	glWalls[i] = newBuffer;
	return newBuffer;
}

void GL_LoadLevelWalls(const Level *l)
{
	GL_DestroyWallBuffers();
	for (size_t i = 0; i < l->walls.length; i++)
	{
		const Wall *w = ListGetPointer(l->walls, i);
		GL_WallBuffers *wallBuffer = GL_GetWallBuffer(w->tex);
		if (wallBuffer->wallCount + 1 > GL_MAX_WALLS_PER_BUFFER)
		{
			Error("Too many walls per buffer");
		}

		float vertices[4][6] = {
			// X Y Z U V A
			{(float)w->a.x, 0.5f, (float)w->a.y, 0.0f, 0.0f, w->angle},
			{(float)w->b.x, 0.5f, (float)w->b.y, (float)w->length, 0.0f, w->angle},
			{(float)w->b.x, -0.5f, (float)w->b.y, (float)w->length, 1.0f, w->angle},
			{(float)w->a.x, -0.5f, (float)w->a.y, 0.0f, 1.0f, w->angle},
		};

		const float uvOffset = w->uvOffset;
		const float uvScale = w->uvScale;
		for (int j = 0; j < 4; j++)
		{
			vertices[j][3] = vertices[j][3] * uvScale + uvOffset;
		}

		uint32_t indices[] = {0, 1, 2, 0, 2, 3};
		for (int j = 0; j < 6; j++)
		{
			indices[j] += wallBuffer->wallCount * 4;
		}

		const GLintptr vertexOffset = (GLintptr)(sizeof(float) * 24 * wallBuffer->wallCount);
		const GLintptr indexOffset = (GLintptr)(sizeof(uint32_t) * 6 * wallBuffer->wallCount);

		glBindBuffer(GL_ARRAY_BUFFER, wallBuffer->buffer->vertexBufferObject);
		glBufferSubData(GL_ARRAY_BUFFER, vertexOffset, sizeof(vertices), vertices);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, wallBuffer->buffer->elementBufferObject);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, indexOffset, sizeof(indices), indices);

		wallBuffer->wallCount++;
	}
}

#pragma endregion


#pragma region Frame Operations

inline void GL_ClearScreen()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

inline void GL_ClearDepthOnly()
{
	glClear(GL_DEPTH_BUFFER_BIT);
}

inline void GL_Swap()
{
	SDL_GL_SwapWindow(GetGameWindow());
}

inline void GL_UpdateViewportSize()
{
	int vpWidth = 0;
	int vpHeight = 0;
	SDL_GL_GetDrawableSize(GetGameWindow(), &vpWidth, &vpHeight);
	glViewport(0, 0, vpWidth, vpHeight);
}

#pragma endregion


#pragma region UI Drawing

void GL_DrawRect(const Vector2 pos, const Vector2 size, const Color color)
{
	glUseProgram(uiColoredShader->program);

	glUniform4fv(hudColoredColorLoc, 1, COLOR_TO_ARR(color));

	const Vector2 ndcStartPos = v2(GL_X_TO_NDC(pos.x), GL_Y_TO_NDC(pos.y));
	const Vector2 ncdEndPos = v2(GL_X_TO_NDC(pos.x + size.x), GL_Y_TO_NDC(pos.y + size.y));


	const float vertices[4][2] = {
		{(float)ndcStartPos.x, (float)ndcStartPos.y},
		{(float)ncdEndPos.x, (float)ndcStartPos.y},
		{(float)ncdEndPos.x, (float)ncdEndPos.y},
		{(float)ndcStartPos.x, (float)ncdEndPos.y},
	};

	const uint32_t indices[] = {0, 2, 1, 0, 3, 2};

	glBindVertexArray(glBuffer->vertexArrayObject);

	glBindBuffer(GL_ARRAY_BUFFER, glBuffer->vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glBuffer->elementBufferObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STREAM_DRAW);

	const GLint posAttrLoc = glGetAttribLocation(uiColoredShader->program, "VERTEX");
	glVertexAttribPointer(posAttrLoc, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void *)0);
	glEnableVertexAttribArray(posAttrLoc);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
}

void GL_DrawRectOutline(const Vector2 pos, const Vector2 size, const Color color, const float thickness)
{
	if (thickness < 1.0f)
	{
		glEnable(GL_LINE_SMOOTH);
	} else
	{
		glDisable(GL_LINE_SMOOTH);
	}

	glLineWidth(thickness);

	glUseProgram(uiColoredShader->program);

	glUniform4fv(hudColoredColorLoc, 1, COLOR_TO_ARR(color));

	const Vector2 ndcStartPos = v2(GL_X_TO_NDC(pos.x), GL_Y_TO_NDC(pos.y));
	const Vector2 ndcEndPos = v2(GL_X_TO_NDC(pos.x + size.x), GL_Y_TO_NDC(pos.y + size.y));


	const float vertices[4][2] = {
		{(float)ndcStartPos.x, (float)ndcStartPos.y},
		{(float)ndcEndPos.x, (float)ndcStartPos.y},
		{(float)ndcEndPos.x, (float)ndcEndPos.y},
		{(float)ndcStartPos.x, (float)ndcEndPos.y},
	};

	const uint32_t indices[] = {0, 1, 2, 3};

	glBindVertexArray(glBuffer->vertexArrayObject);

	glBindBuffer(GL_ARRAY_BUFFER, glBuffer->vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glBuffer->elementBufferObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STREAM_DRAW);

	const GLint posAttrLoc = glGetAttribLocation(uiColoredShader->program, "VERTEX");
	glVertexAttribPointer(posAttrLoc, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void *)0);
	glEnableVertexAttribArray(posAttrLoc);

	glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_INT, NULL);
}

void GL_DrawTexture_Internal(const Vector2 pos,
							 const Vector2 size,
							 const char *texture,
							 const Color color,
							 const Vector2 regionStart,
							 const Vector2 regionEnd)
{
	glUseProgram(uiTexturedShader->program);

	GL_LoadTextureFromAsset(texture);

	glUniform4fv(hudTexturedColorLoc, 1, COLOR_TO_ARR(color));

	glUniform4f(hudTexturedRegionLoc,
				(GLfloat)regionStart.x,
				(GLfloat)regionStart.y,
				(GLfloat)regionEnd.x,
				(GLfloat)regionEnd.y);

	const Vector2 ndcStartPos = v2(GL_X_TO_NDC(pos.x), GL_Y_TO_NDC(pos.y));
	const Vector2 ndcEndPos = v2(GL_X_TO_NDC(pos.x + size.x), GL_Y_TO_NDC(pos.y + size.y));


	const float vertices[4][4] = {
		{(float)ndcStartPos.x, (float)ndcStartPos.y, 0.0f, 0.0f},
		{(float)ndcEndPos.x, (float)ndcStartPos.y, 1.0f, 0.0f},
		{(float)ndcEndPos.x, (float)ndcEndPos.y, 1.0f, 1.0f},
		{(float)ndcStartPos.x, (float)ndcEndPos.y, 0.0f, 1.0f},
	};

	const uint32_t indices[] = {0, 2, 1, 0, 3, 2};

	glBindVertexArray(glBuffer->vertexArrayObject);

	glBindBuffer(GL_ARRAY_BUFFER, glBuffer->vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glBuffer->elementBufferObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STREAM_DRAW);

	const GLint posAttrLoc = glGetAttribLocation(uiTexturedShader->program, "VERTEX");
	glVertexAttribPointer(posAttrLoc, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void *)0);
	glEnableVertexAttribArray(posAttrLoc);

	const GLint texAttrLoc = glGetAttribLocation(uiTexturedShader->program, "VERTEX_UV");
	glVertexAttribPointer(texAttrLoc, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void *)(2 * sizeof(GLfloat)));
	glEnableVertexAttribArray(texAttrLoc);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
}

inline void GL_DrawTexture(const Vector2 pos, const Vector2 size, const char *texture)
{
	GL_DrawTexture_Internal(pos, size, texture, COLOR_WHITE, v2(-1, 0), v2s(0));
}

inline void GL_DrawTextureMod(const Vector2 pos, const Vector2 size, const char *texture, const Color color)
{
	GL_DrawTexture_Internal(pos, size, texture, color, v2(-1, 0), v2s(0));
}

inline void GL_DrawTextureRegion(const Vector2 pos,
								 const Vector2 size,
								 const char *texture,
								 const Vector2 regionStart,
								 const Vector2 regionEnd)
{
	GL_DrawTexture_Internal(pos, size, texture, COLOR_WHITE, regionStart, regionEnd);
}

inline void GL_DrawTextureRegionMod(const Vector2 pos,
									const Vector2 size,
									const char *texture,
									const Vector2 regionStart,
									const Vector2 regionEnd,
									const Color color)
{
	GL_DrawTexture_Internal(pos, size, texture, color, regionStart, regionEnd);
}

void GL_DrawLine(const Vector2 start, const Vector2 end, const Color color, const float thickness)
{
	if (thickness < 1.0f)
	{
		glEnable(GL_LINE_SMOOTH);
	} else
	{
		glDisable(GL_LINE_SMOOTH);
	}

	glUseProgram(uiColoredShader->program);

	glUniform4fv(hudColoredColorLoc, 1, COLOR_TO_ARR(color));

	const Vector2 ndcStartPos = v2(GL_X_TO_NDC(start.x), GL_Y_TO_NDC(start.y));
	const Vector2 ndcEndPos = v2(GL_X_TO_NDC(end.x), GL_Y_TO_NDC(end.y));

	// Calculate the 2 corner vertices of each point for a thick line
	const float vertices[2][2] = {
		{(float)ndcStartPos.x, (float)ndcStartPos.y},
		{(float)ndcEndPos.x, (float)ndcEndPos.y},
	};

	const uint32_t indices[] = {0, 1};

	glBindVertexArray(glBuffer->vertexArrayObject);

	glBindBuffer(GL_ARRAY_BUFFER, glBuffer->vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glBuffer->elementBufferObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STREAM_DRAW);

	const GLint posAttrLoc = glGetAttribLocation(uiColoredShader->program, "VERTEX");
	glVertexAttribPointer(posAttrLoc, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void *)0);
	glEnableVertexAttribArray(posAttrLoc);

	glLineWidth(thickness);
	glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, NULL);
}

void GL_DrawColoredArrays(const float *vertices, const uint32_t *indices, const uint32_t quadCount, const Color color)
{
	glUseProgram(uiColoredShader->program);

	glUniform4fv(hudColoredColorLoc, 1, COLOR_TO_ARR(color));

	glBindVertexArray(glBuffer->vertexArrayObject);

	glBindBuffer(GL_ARRAY_BUFFER, glBuffer->vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, (long)(quadCount * 16 * sizeof(float)), vertices, GL_STREAM_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glBuffer->elementBufferObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, (long)(quadCount * 6 * sizeof(uint32_t)), indices, GL_STREAM_DRAW);

	const GLint posAttrLoc = glGetAttribLocation(uiColoredShader->program, "VERTEX");
	glVertexAttribPointer(posAttrLoc, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void *)0);
	glEnableVertexAttribArray(posAttrLoc);

	glDrawElements(GL_TRIANGLES, (int)(quadCount * 6), GL_UNSIGNED_INT, NULL);
}

void GL_DrawTexturedArrays(const float *vertices,
						   const uint32_t *indices,
						   const int quadCount,
						   const char *texture,
						   const Color color)
{
	glUseProgram(uiTexturedShader->program);

	GL_LoadTextureFromAsset(texture);

	glUniform4fv(hudTexturedColorLoc, 1, COLOR_TO_ARR(color));

	glUniform4f(hudTexturedRegionLoc, -1, 0, 0, 0);

	glBindVertexArray(glBuffer->vertexArrayObject);

	glBindBuffer(GL_ARRAY_BUFFER, glBuffer->vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, (long)(quadCount * 16 * sizeof(float)), vertices, GL_STREAM_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glBuffer->elementBufferObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, (long)(quadCount * 6 * sizeof(uint32_t)), indices, GL_STREAM_DRAW);

	const GLint posAttrLoc = glGetAttribLocation(uiTexturedShader->program, "VERTEX");
	glVertexAttribPointer(posAttrLoc, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void *)0);
	glEnableVertexAttribArray(posAttrLoc);

	const GLint texAttrLoc = glGetAttribLocation(uiTexturedShader->program, "VERTEX_UV");
	glVertexAttribPointer(texAttrLoc, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void *)(2 * sizeof(GLfloat)));
	glEnableVertexAttribArray(texAttrLoc);

	glDrawElements(GL_TRIANGLES, quadCount * 6, GL_UNSIGNED_INT, NULL);
}

void GL_DrawUITriangles(const UiTriangleArray *tris, const char *texture, const Color col)
{
	glUseProgram(uiTexturedShader->program);

	GL_LoadTextureFromAsset(texture);

	glUniform4fv(hudTexturedColorLoc, 1, COLOR_TO_ARR(col));

	glUniform4f(hudTexturedRegionLoc, -1, 0, 0, 0);

	glBindVertexArray(glBuffer->vertexArrayObject);

	glBindBuffer(GL_ARRAY_BUFFER, glBuffer->vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, (long)(tris->vertexCount * 4 * sizeof(float)), tris->vertices, GL_STREAM_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glBuffer->elementBufferObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, (long)(tris->indexCount * sizeof(uint32_t)), tris->indices, GL_STREAM_DRAW);

	const GLint posAttrLoc = glGetAttribLocation(uiTexturedShader->program, "VERTEX");
	glVertexAttribPointer(posAttrLoc, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void *)0);
	glEnableVertexAttribArray(posAttrLoc);

	const GLint texAttrLoc = glGetAttribLocation(uiTexturedShader->program, "VERTEX_UV");
	glVertexAttribPointer(texAttrLoc, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void *)(2 * sizeof(GLfloat)));
	glEnableVertexAttribArray(texAttrLoc);

	glDrawElements(GL_TRIANGLES, (GLsizei)tris->indexCount, GL_UNSIGNED_INT, NULL);
}

#pragma endregion


#pragma region World Utilities

void GL_SetLevelParams(mat4 *modelViewProjection, const Level *level)
{
	GL_SharedUniforms uniforms;
	glm_mat4_copy(*modelViewProjection, uniforms.worldViewMatrix);
	uniforms.fogColor = COLOR(level->fogColor);
	uniforms.cameraYaw = JPH_Quat_GetRotationAngle(&GetState()->camera->transform.rotation, &Vector3_AxisY);
	uniforms.fogStart = (float)level->fogStart;
	uniforms.fogEnd = (float)level->fogEnd;

	glBindBuffer(GL_UNIFORM_BUFFER, sharedUniformBuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(GL_SharedUniforms), &uniforms, GL_STREAM_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

inline void GL_Enable3D()
{
	glDisable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
	glClear(GL_DEPTH_BUFFER_BIT);
}

inline void GL_Disable3D()
{
	glEnable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_MULTISAMPLE);
	glClear(GL_DEPTH_BUFFER_BIT);
}

void GL_GetMatrix(const Camera *camera, mat4 *modelViewProjectionMatrix)
{
	mat4 perspectiveMatrix;
	glm_perspective(glm_rad(camera->fov), WindowWidthFloat() / WindowHeightFloat(), NEAR_Z, FAR_Z, perspectiveMatrix);

	versor rotationQuat;
	QUAT_TO_VERSOR(camera->transform.rotation, rotationQuat);

	mat4 viewMatrix;
	glm_quat_look(VECTOR3_TO_VEC3(camera->transform.position), rotationQuat, viewMatrix);

	glm_mat4_mul(perspectiveMatrix, viewMatrix, *modelViewProjectionMatrix);
}

void GL_GetViewmodelMatrix(mat4 *out)
{
	mat4 perspectiveMatrix;
	glm_perspective(glm_rad(VIEWMODEL_FOV), WindowWidthFloat() / WindowHeightFloat(), NEAR_Z, FAR_Z, perspectiveMatrix);

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

#pragma endregion


#pragma region World Drawing

void GL_DrawActorWall(const Actor *actor, const mat4 actorXfm)
{
	const ActorWall *wall = actor->actorWall;

	glUseProgram(wallShader->program);

	glBindBufferBase(GL_UNIFORM_BUFFER, wallSharedUniformsLoc, sharedUniformBuffer);

	glUniformMatrix4fv(wallTransformMatrixLoc, 1, GL_FALSE, *actorXfm);

	GL_LoadTextureFromAsset(wall->tex);

	const float halfHeight = wall->height / 2.0f;
	const Vector2 startVertex = v2(wall->a.x, wall->a.y);
	const Vector2 endVertex = v2(wall->b.x, wall->b.y);
	const Vector2 startUV = v2(wall->uvOffset, 0);
	const Vector2 endUV = v2(wall->uvScale * wall->length + wall->uvOffset, 1);
	const float vertices[4][6] = {
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
	};

	const uint32_t indices[] = {0, 1, 2, 0, 2, 3};

	glBindVertexArray(glBuffer->vertexArrayObject);

	glBindBuffer(GL_ARRAY_BUFFER, glBuffer->vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glBuffer->elementBufferObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STREAM_DRAW);

	const GLint posAttrLoc = glGetAttribLocation(wallShader->program, "VERTEX");
	glVertexAttribPointer(posAttrLoc, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void *)0);
	glEnableVertexAttribArray(posAttrLoc);

	const GLint texAttrLoc = glGetAttribLocation(wallShader->program, "VERTEX_UV");
	glVertexAttribPointer(texAttrLoc, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void *)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(texAttrLoc);

	const GLint angleAttrLoc = glGetAttribLocation(wallShader->program, "VERTEX_ANGLE");
	glVertexAttribPointer(angleAttrLoc, 1, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void *)(5 * sizeof(GLfloat)));
	glEnableVertexAttribArray(angleAttrLoc);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
}

void GL_DrawFloor(const Vector2 vp1, const Vector2 vp2, const char *texture, const float height, const float shade)
{
	glUseProgram(floorAndCeilingShader->program);

	glBindBufferBase(GL_UNIFORM_BUFFER, floorSharedUniformsLoc, sharedUniformBuffer);

	GL_LoadTextureFromAsset(texture);

	glUniform1f(floorHeightLoc, height);
	glUniform1f(floorShadeLoc, shade);

	const float vertices[4][2] = {
		// X Z
		{(float)vp1.x, (float)vp1.y},
		{(float)vp2.x, (float)vp1.y},
		{(float)vp2.x, (float)vp2.y},
		{(float)vp1.x, (float)vp2.y},
	};

	const uint32_t indices[] = {0, 1, 2, 0, 2, 3};

	glBindVertexArray(glBuffer->vertexArrayObject);

	glBindBuffer(GL_ARRAY_BUFFER, glBuffer->vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glBuffer->elementBufferObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STREAM_DRAW);

	const GLint posAttrLoc = glGetAttribLocation(floorAndCeilingShader->program, "VERTEX");
	glVertexAttribPointer(posAttrLoc, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void *)0);
	glEnableVertexAttribArray(posAttrLoc);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
}

void GL_RenderLevel(const Level *level, const Camera *camera)
{
	GL_Enable3D();

	//glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	//glLineWidth(2);

	mat4 worldViewMatrix;
	GL_GetMatrix(camera, &worldViewMatrix);
	mat4 skyModelWorldMatrix = GLM_MAT4_IDENTITY_INIT;
	glm_translated(skyModelWorldMatrix, VECTOR3_TO_VEC3(camera->transform.position));

	const Vector2 floorStart = v2(level->player.transform.position.x - 100, level->player.transform.position.z - 100);
	const Vector2 floorEnd = v2(level->player.transform.position.x + 100, level->player.transform.position.z + 100);

	GL_SetLevelParams(&worldViewMatrix, level);

	if (level->hasCeiling)
	{
		GL_DrawFloor(floorStart, floorEnd, level->ceilOrSkyTex, 0.5f, 0.8f);
	} else
	{
		GL_RenderModel(LoadModel(MODEL("sky")), skyModelWorldMatrix, 0, 0, COLOR_WHITE);
		GL_ClearDepthOnly(); // prevent sky from clipping into walls
	}

	GL_DrawFloor(floorStart, floorEnd, level->floorTex, -0.5f, 1.0f);

	GL_RenderLevelWalls();

	for (size_t i = 0; i < level->actors.length; i++)
	{
		const Actor *actor = ListGetPointer(level->actors, i);
		if (!actor->actorWall && !actor->actorModel)
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
			GL_DrawActorWall(actor, actorXfm);
		} else
		{
			GL_RenderModel(actor->actorModel, actorXfm, actor->currentSkinIndex, actor->currentLod, actor->modColor);
		}
	}

	if (GetState()->viewmodel.enabled)
	{
		glClear(GL_DEPTH_BUFFER_BIT);

		mat4 viewModelMatrix;
		GL_GetViewmodelMatrix(&viewModelMatrix);

		GL_SharedUniforms uniforms = {
			.fogStart = 1000,
			.fogEnd = 1001,
		};
		glm_mat4_copy(viewModelMatrix, uniforms.worldViewMatrix);

		glBindBuffer(GL_UNIFORM_BUFFER, sharedUniformBuffer);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(GL_SharedUniforms), &uniforms, GL_STREAM_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		GL_RenderModel(GetState()->viewmodel.model, GLM_MAT4_IDENTITY, 0, 0, COLOR_WHITE);
	}

	//glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
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
		GL_LoadTextureFromAsset(GetState()->level->ceilOrSkyTex);
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
	glEnable(GL_CULL_FACE);
	for (uint32_t material = 0; material < model->materialsPerSkin; material++)
	{
		GL_RenderModelPart(model, modelWorldMatrix, lod, material, skin, modColor);
	}
	glDisable(GL_CULL_FACE);
}

void GL_RenderLevelWalls()
{
	glUseProgram(wallShader->program);

	glBindBufferBase(GL_UNIFORM_BUFFER, wallSharedUniformsLoc, sharedUniformBuffer);

	glUniformMatrix4fv(wallTransformMatrixLoc, 1, GL_FALSE, *GLM_MAT4_IDENTITY);

	size_t i = 0;
	while (glWalls[i] != NULL && i < GL_MAX_WALL_BUFFERS)
	{
		const GL_WallBuffers *wallBuffer = glWalls[i];

		GL_LoadTextureFromAsset(wallBuffer->texture);

		glBindVertexArray(wallBuffer->buffer->vertexArrayObject);

		glBindBuffer(GL_ARRAY_BUFFER, wallBuffer->buffer->vertexBufferObject);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, wallBuffer->buffer->elementBufferObject);

		const GLint posAttrLoc = glGetAttribLocation(wallShader->program, "VERTEX");
		glVertexAttribPointer(posAttrLoc, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void *)0);
		glEnableVertexAttribArray(posAttrLoc);

		const GLint texAttrLoc = glGetAttribLocation(wallShader->program, "VERTEX_UV");
		glVertexAttribPointer(texAttrLoc, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void *)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(texAttrLoc);

		const GLint angleAttrLoc = glGetAttribLocation(wallShader->program, "VERTEX_ANGLE");
		glVertexAttribPointer(angleAttrLoc, 1, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void *)(5 * sizeof(GLfloat)));
		glEnableVertexAttribArray(angleAttrLoc);

		glDrawElements(GL_TRIANGLES, (GLsizei)wallBuffer->wallCount * 6, GL_UNSIGNED_INT, NULL);

		i++;
	}
}

#pragma endregion

void GL_Error(const char *error)
{
	LogError("OpenGL Error: %s\n", error);
	strcpy(glLastError, error);
}
