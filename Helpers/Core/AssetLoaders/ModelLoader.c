//
// Created by droc101 on 7/23/25.
//

#include "ModelLoader.h"
#include "../Error.h"
#include "../AssetReader.h"
#include "../DataReader.h"
#include "../Logging.h"
#include "../../Graphics/RenderingHelpers.h"

size_t modelId;
size_t lodId;
size_t materialId;
ModelDefinition *models[MAX_MODELS];
ModelDefinition *errorModel = NULL;

void InitModelLoader()
{
	errorModel = LoadModel(MODEL("error"));
}

ModelDefinition *LoadModelInternal(const char* asset)
{
	Asset *assetData = DecompressAsset(asset, false);
	if (assetData == NULL)
	{
		LogError("Failed to load model from asset, asset was NULL!\n");
		return NULL;
	}
	if (assetData->typeVersion != MODEL_ASSET_VERSION)
	{
		LogError("Failed to load model from asset due to version mismatch (got %d, expected %d)",
				 assetData->typeVersion,
				 MODEL_ASSET_VERSION);
		return NULL;
	}
	ModelDefinition *model = malloc(sizeof(ModelDefinition));
	CheckAlloc(model);

	model->id = modelId;
	models[modelId] = model;
	modelId++;

	const size_t nameLength = strlen(asset) + 1;
	model->name = malloc(nameLength);
	CheckAlloc(model->name);
	strncpy(model->name, asset, nameLength);

	size_t offset = 0;
	model->materialCount = ReadSizeT(assetData->data, &offset);
	model->materialsPerSkin = ReadSizeT(assetData->data, &offset);
	model->skinCount = ReadSizeT(assetData->data, &offset);
	model->lodCount = ReadSizeT(assetData->data, &offset);
	offset += sizeof(uint8_t); // skip collision model type as it is currently unused

	model->materials = malloc(sizeof(Material) * model->materialCount);
	CheckAlloc(model->materials);
	for (size_t i = 0; i < model->materialCount; i++)
	{
		Material *mat = &model->materials[i];
		size_t l;
		mat->texture = ReadStringSafe(assetData->data, &offset, assetData->size, &l);
		mat->color.r = ReadFloat(assetData->data, &offset);
		mat->color.g = ReadFloat(assetData->data, &offset);
		mat->color.b = ReadFloat(assetData->data, &offset);
		mat->color.a = ReadFloat(assetData->data, &offset);
		mat->shader = ReadUint(assetData->data, &offset);
	}

	model->skins = malloc(sizeof(size_t) * model->skinCount);
	CheckAlloc(model->skins);

	const size_t skinSize = sizeof(size_t) * model->materialsPerSkin;
	for (int i = 0; i < model->skinCount; i++)
	{
		model->skins[i] = malloc(skinSize);
		CheckAlloc(model->skins[i]);
		size_t *skin = model->skins[i];
		for (int j = 0; j < model->materialsPerSkin; j++)
		{
			skin[j] = ReadSizeT(assetData->data, &offset);
		}
	}

	model->lods = malloc(sizeof(ModelLod *) * model->lodCount);
	CheckAlloc(model->lods);
	for (int i = 0; i < model->lodCount; i++)
	{
		model->lods[i] = malloc(sizeof(ModelLod));
		CheckAlloc(model->lods[i]);
		ModelLod *lod = model->lods[i];

		lod->id = lodId;
		lodId++;

		offset += sizeof(float); // skip non-squared lod distance
		lod->distanceSquared = ReadFloat(assetData->data, &offset);
		lod->vertexCount = ReadSizeT(assetData->data, &offset);

		const size_t vertexDataSize = lod->vertexCount * sizeof(float) * 12;
		lod->vertexData = malloc(vertexDataSize);
		CheckAlloc(lod->vertexData);
		ReadBytes(assetData->data, &offset, vertexDataSize, lod->vertexData);

		lod->totalIndexCount = ReadUint(assetData->data, &offset);
		const size_t indexCountSize = model->materialsPerSkin * sizeof(uint);
		lod->indexCount = malloc(indexCountSize);
		CheckAlloc(lod->indexCount);
		ReadBytes(assetData->data, &offset, indexCountSize, lod->indexCount);

		lod->indexData = malloc(sizeof(uint *) * model->materialsPerSkin);
		CheckAlloc(lod->indexData);
		for (int j = 0; j < model->materialsPerSkin; j++)
		{
			uint *indexData = malloc(lod->indexCount[j] * sizeof(uint));
			CheckAlloc(indexData);
			lod->indexData[j] = indexData;
			ReadBytes(assetData->data, &offset, lod->indexCount[j] * sizeof(uint), indexData);
		}
	}

	FreeAsset(assetData);

	return model;
}

ModelDefinition *LoadModel(const char *asset)
{
	for (int i = 0; i < MAX_MODELS; i++)
	{
		ModelDefinition *model = models[i];
		if (model == NULL)
		{
			break;
		}
		if (strncmp(asset, model->name, 80) == 0)
		{
			return model;
		}
	}

	if (modelId >= MAX_MODELS)
	{
		Error("Model ID heap exhausted. Please increase MAX_MODELS\n");
	}

	ModelDefinition *model = LoadModelInternal(asset);
	if (model == NULL)
	{
		if (errorModel != NULL)
		{
			model = errorModel;
		} else
		{
			Error("Failed to load a model and could not find an error model.\n");
		}
	}

	if (modelId >= MAX_MODELS - 10)
	{
		LogWarning("Model ID heap is nearly exhausted! Only %lu slots remain.\n", MAX_MODELS - modelId);
	}

	return model;
}

inline ModelDefinition *GetModelFromId(const uint id)
{
	if (id >= modelId)
	{
		Error("Invalid model ID!\n");
	}

	return models[id];
}

void FreeModel(ModelDefinition *model)
{
	if (model == NULL)
	{
		return;
	}
	for (int i = 0; i < model->skinCount; i++)
	{
		free(model->skins[i]);
	}

	for (int i = 0; i < model->lodCount; i++)
	{
		ModelLod *lod = model->lods[i];
		free(lod->vertexData);
		for (int j = 0; j < model->materialCount; j++)
		{
			free(lod->indexData[j]);
		}
		free(lod->indexData);
		free(lod->indexCount);
		free(lod);
	}

	for (int i = 0; i < model->materialCount; i++)
	{
		free(model->materials[i].texture);
	}

	free(model->name);
	free(model->skins);
	free(model->lods);
	free(model->materials);
	free(model);

	model = NULL;
}

void DestroyModelLoader()
{
	for (int i = 0; i < MAX_MODELS; i++)
	{
		FreeModel(models[i]);
	}
}
