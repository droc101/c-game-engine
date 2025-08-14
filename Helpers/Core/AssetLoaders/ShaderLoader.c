//
// Created by droc101 on 7/24/25.
//

#include "ShaderLoader.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include "../AssetReader.h"
#include "../DataReader.h"
#include "../Error.h"
#include "../Logging.h"

Shader *LoadShader(const char *asset)
{
	Asset *assetData = DecompressAsset(asset, false);
	if (assetData == NULL)
	{
		LogError("Failed to load shader from asset, asset was NULL!\n");
		return NULL;
	}
	if (assetData->typeVersion != SHADER_ASSET_VERSION)
	{
		LogError("Failed to load shader from asset due to version mismatch (got %d, expected %d)\n",
				 assetData->typeVersion,
				 SHADER_ASSET_VERSION);
		return NULL;
	}
	Shader *shader = malloc(sizeof(Shader));
	CheckAlloc(shader);
	size_t offset = 0;
	shader->platform = ReadByte(assetData->data, &offset);
	shader->type = ReadByte(assetData->data, &offset);
	shader->glslLength = ReadSizeT(assetData->data, &offset);
	shader->glsl = calloc(shader->glslLength, sizeof(char));
	ReadBytes(assetData->data, &offset, shader->glslLength * sizeof(char), shader->glsl);
	shader->spirvLength = ReadSizeT(assetData->data, &offset);
	shader->spirv = calloc(shader->spirvLength, sizeof(uint32_t));
	ReadBytes(assetData->data, &offset, shader->spirvLength * sizeof(uint32_t), shader->spirv);
	FreeAsset(assetData);

	return shader;
}

void FreeShader(Shader *shader)
{
	free(shader->glsl);
	free(shader->spirv);
	free(shader);
}
