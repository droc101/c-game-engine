//
// Created by droc101 on 7/24/25.
//

#include <engine/assets/AssetReader.h>
#include <engine/assets/DataReader.h>
#include <engine/assets/ShaderLoader.h>
#include <engine/structs/Asset.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Logging.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

Shader *LoadShader(const char *asset)
{
	Asset *assetData = LoadAsset(asset, false, true);
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
	DataReader *reader = CreateDataReaderFromAsset(assetData);
	size_t bytesRemaining = assetData->size;
	EXPECT_BYTES(2 + sizeof(size_t), bytesRemaining);
	shader->platform = ReadUint8(reader);
	shader->type = ReadUint8(reader);
	shader->glslLength = ReadSizeT(reader);
	EXPECT_BYTES(shader->glslLength, bytesRemaining);
	shader->glsl = calloc(shader->glslLength, sizeof(char));
	CheckAlloc(shader->glsl);
	ReadBuffer(reader, shader->glslLength * sizeof(char), shader->glsl);
	EXPECT_BYTES(sizeof(size_t), bytesRemaining);
	shader->spirvLength = ReadSizeT(reader);
	EXPECT_BYTES(shader->spirvLength * sizeof(uint32_t), bytesRemaining);
	shader->spirv = calloc(shader->spirvLength, sizeof(uint32_t));
	CheckAlloc(shader->spirv);
	ReadBuffer(reader, shader->spirvLength * sizeof(uint32_t), shader->spirv);
	DestroyDataReader(reader);
	FreeAsset(assetData);

	return shader;
}

void FreeShader(Shader *shader)
{
	free(shader->glsl);
	free(shader->spirv);
	free(shader);
}
