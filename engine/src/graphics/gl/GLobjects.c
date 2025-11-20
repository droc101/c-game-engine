//
// Created by droc101 on 11/20/25.
//

#include <cglm/cglm.h>
#include <engine/assets/ModelLoader.h>
#include <engine/assets/ShaderLoader.h>
#include <engine/assets/TextureLoader.h>
#include <engine/graphics/gl/GLobjects.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/Options.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Logging.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

GLuint glTextures[MAX_TEXTURES];
int glNextFreeSlot = 0;
int glAssetTextureMap[MAX_TEXTURES];
char glLastError[512];

GL_ModelBuffers *glModels[MAX_MODELS];

GL_Buffer *glBuffer;

GLuint sharedUniformBuffer;

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

void GL_InitObjects()
{
	memset(glAssetTextureMap, -1, MAX_TEXTURES * sizeof(int));
	memset(glTextures, 0, sizeof(glTextures));

	glBuffer = GL_ConstructBuffer();

	glGenBuffers(1, &sharedUniformBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, sharedUniformBuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(GL_SharedUniforms), NULL, GL_STREAM_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void GL_DestroyObjects()
{
	GL_DestroyBuffer(glBuffer);
	glDeleteBuffers(1, &sharedUniformBuffer);
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
}
