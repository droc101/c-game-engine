//
// Created by droc101 on 7/23/25.
//

#include "ModelLoader.h"
#include <joltc/constants.h>
#include <joltc/joltc.h>
#include <joltc/Math/Quat.h>
#include <joltc/Math/Vector3.h>
#include <joltc/Physics/Collision/Shape/Shape.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "../../../Structs/Asset.h"
#include "../AssetReader.h"
#include "../DataReader.h"
#include "../Error.h"
#include "../Logging.h"

uint32_t modelId;
uint32_t lodId;
ModelDefinition *models[MAX_MODELS];
ModelDefinition *errorModel = NULL;

#define BOUNDING_BOX_CONVEX_RADIUS 0.0005f

void InitModelLoader()
{
	errorModel = LoadModel(MODEL("error"));
}

ModelDefinition *LoadModelInternal(const char *asset)
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
	model->materialCount = ReadUint(assetData->data, &offset);
	model->materialsPerSkin = ReadUint(assetData->data, &offset);
	model->skinCount = ReadUint(assetData->data, &offset);
	model->lodCount = ReadUint(assetData->data, &offset);
	model->collisionModelType = ReadByte(assetData->data, &offset);

	model->materials = malloc(sizeof(Material) * model->materialCount);
	CheckAlloc(model->materials);
	for (uint32_t i = 0; i < model->materialCount; i++)
	{
		Material *mat = &model->materials[i];
		size_t _ = 0;
		mat->texture = ReadStringSafe(assetData->data, &offset, assetData->size, &_);
		mat->color.r = ReadFloat(assetData->data, &offset);
		mat->color.g = ReadFloat(assetData->data, &offset);
		mat->color.b = ReadFloat(assetData->data, &offset);
		mat->color.a = ReadFloat(assetData->data, &offset);
		mat->shader = ReadUint(assetData->data, &offset);
	}

	model->skins = malloc(sizeof(uint32_t *) * model->skinCount);
	CheckAlloc(model->skins);

	const size_t skinSize = sizeof(uint32_t) * model->materialsPerSkin;
	for (uint32_t i = 0; i < model->skinCount; i++)
	{
		model->skins[i] = malloc(skinSize);
		CheckAlloc(model->skins[i]);
		uint32_t *skin = model->skins[i];
		for (uint32_t j = 0; j < model->materialsPerSkin; j++)
		{
			skin[j] = ReadUint(assetData->data, &offset);
		}
	}

	model->lods = malloc(sizeof(ModelLod *) * model->lodCount);
	CheckAlloc(model->lods);
	for (uint32_t i = 0; i < model->lodCount; i++)
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
		const size_t indexCountSize = model->materialsPerSkin * sizeof(uint32_t);
		lod->indexCount = malloc(indexCountSize);
		CheckAlloc(lod->indexCount);
		ReadBytes(assetData->data, &offset, indexCountSize, lod->indexCount);

		lod->indexData = malloc(sizeof(uint32_t *) * model->materialsPerSkin);
		CheckAlloc(lod->indexData);
		for (uint32_t j = 0; j < model->materialsPerSkin; j++)
		{
			uint32_t *indexData = malloc(lod->indexCount[j] * sizeof(uint32_t));
			CheckAlloc(indexData);
			lod->indexData[j] = indexData;
			ReadBytes(assetData->data, &offset, lod->indexCount[j] * sizeof(uint32_t), indexData);
		}
	}

	model->boundingBoxOrigin.x = ReadFloat(assetData->data, &offset);
	model->boundingBoxOrigin.y = ReadFloat(assetData->data, &offset);
	model->boundingBoxOrigin.z = ReadFloat(assetData->data, &offset);
	model->boundingBoxExtents.x = ReadFloat(assetData->data, &offset);
	model->boundingBoxExtents.y = ReadFloat(assetData->data, &offset);
	model->boundingBoxExtents.z = ReadFloat(assetData->data, &offset);
	model->boundingBoxShape = (JPH_Shape *)JPH_BoxShape_Create(&model->boundingBoxExtents, BOUNDING_BOX_CONVEX_RADIUS);

	if (model->collisionModelType == COLLISION_MODEL_TYPE_DYNAMIC)
	{
		const size_t numHulls = ReadSizeT(assetData->data, &offset);
		ModelConvexHull *hulls = malloc(sizeof(ModelConvexHull) * numHulls);
		CheckAlloc(hulls);
		for (size_t i = 0; i < numHulls; i++)
		{
			ModelConvexHull *hull = &hulls[i];
			hull->numPoints = ReadSizeT(assetData->data, &offset);
			hull->offset.x = ReadFloat(assetData->data, &offset);
			hull->offset.y = ReadFloat(assetData->data, &offset);
			hull->offset.z = ReadFloat(assetData->data, &offset);
			hull->points = malloc(sizeof(Vector3) * hull->numPoints);
			CheckAlloc(hull->points);
			for (size_t p = 0; p < hull->numPoints; p++)
			{
				Vector3 *point = &hull->points[p];
				point->x = ReadFloat(assetData->data, &offset);
				point->y = ReadFloat(assetData->data, &offset);
				point->z = ReadFloat(assetData->data, &offset);
			}
		}
		model->collisionModelShape = CreateDynamicModelShape(numHulls, hulls);
		for (size_t i = 0; i < numHulls; i++)
		{
			free(hulls[i].points);
		}
		free(hulls);
	} else if (model->collisionModelType == COLLISION_MODEL_TYPE_STATIC)
	{
		ModelStaticCollider staticCollider;
		staticCollider.numTriangles = ReadSizeT(assetData->data, &offset);
		staticCollider.tris = malloc(sizeof(JPH_Triangle) * staticCollider.numTriangles);
		CheckAlloc(staticCollider.tris);
		for (size_t i = 0; i < staticCollider.numTriangles; i++)
		{
			JPH_Triangle *triangle = &staticCollider.tris[i];
			triangle->materialIndex = 0;
			Vector3 *verts[3] = {&triangle->v1, &triangle->v2, &triangle->v3};
			for (int v = 0; v < 3; v++)
			{
				Vector3 *point = verts[v];
				point->x = ReadFloat(assetData->data, &offset);
				point->y = ReadFloat(assetData->data, &offset);
				point->z = ReadFloat(assetData->data, &offset);
			}
		}
		model->collisionModelShape = CreateStaticModelShape(&staticCollider);
		free(staticCollider.tris);
	} else
	{
		model->collisionModelShape = NULL;
	}

	FreeAsset(assetData);

	return model;
}

ModelDefinition *LoadModel(const char *asset)
{
	if (modelId >= MAX_MODELS)
	{
		Error("Model ID heap exhausted. Please increase MAX_MODELS\n");
	}

	for (uint32_t i = 0; i < modelId; i++)
	{
		ModelDefinition *model = models[i];
		if (model == NULL)
		{
			continue;
		}
		if (strncmp(asset, model->name, 80) == 0)
		{
			return model;
		}
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

inline ModelDefinition *GetModelFromId(const size_t id)
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
	for (uint32_t i = 0; i < model->skinCount; i++)
	{
		free(model->skins[i]);
	}

	for (uint32_t i = 0; i < model->lodCount; i++)
	{
		ModelLod *lod = model->lods[i];
		free(lod->vertexData);
		for (uint32_t j = 0; j < model->materialsPerSkin; j++)
		{
			free(lod->indexData[j]);
		}
		free(lod->indexData);
		free(lod->indexCount);
		free(lod);
	}

	for (uint32_t i = 0; i < model->materialsPerSkin; i++)
	{
		free(model->materials[i].texture);
	}

	if (model->collisionModelType != COLLISION_MODEL_TYPE_NONE)
	{
		JPH_Shape_Destroy(model->collisionModelShape);
	}
	JPH_Shape_Destroy(model->boundingBoxShape);

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
JPH_Shape *CreateDynamicModelShape(const size_t numHulls, const ModelConvexHull *hulls)
{
	JPH_StaticCompoundShapeSettings *compoundShapeSettings = JPH_StaticCompoundShapeSettings_Create();
	for (size_t i = 0; i < numHulls; i++)
	{
		const ModelConvexHull *hull = &hulls[i];
		JPH_Shape *hullShape = (JPH_Shape *)JPH_ConvexHullShape_Create(hull->points,
																	   hull->numPoints,
																	   JPH_DefaultConvexRadius);
		JPH_CompoundShapeSettings_AddShape2((JPH_CompoundShapeSettings *)compoundShapeSettings,
											&Vector3_Zero,
											&JPH_Quat_Identity,
											hullShape,
											0);
		JPH_Shape_Destroy(hullShape);
	}
	JPH_Shape *shape = (JPH_Shape *)JPH_StaticCompoundShape_Create(compoundShapeSettings);
	JPH_ShapeSettings_Destroy((JPH_ShapeSettings *)compoundShapeSettings);
	return shape;
}

inline JPH_Shape *CreateStaticModelShape(const ModelStaticCollider *staticCollider)
{
	JPH_MeshShapeSettings *settings = JPH_MeshShapeSettings_Create(staticCollider->tris, staticCollider->numTriangles);
	JPH_Shape *meshShape = (JPH_Shape *)JPH_MeshShapeSettings_CreateShape(settings);
	JPH_ShapeSettings_Destroy((JPH_ShapeSettings *)settings);
	return meshShape;
}
