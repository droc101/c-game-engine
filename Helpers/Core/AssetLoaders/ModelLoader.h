//
// Created by droc101 on 7/23/25.
//

#ifndef MODELLOADER_H
#define MODELLOADER_H

#include <joltc/enums.h>
#include <joltc/joltc.h>
#include <joltc/Math/Transform.h>
#include <joltc/Math/Vector3.h>
#include <joltc/Physics/Body/BodyCreationSettings.h>
#include <joltc/Physics/Collision/Shape/Shape.h>
#include <joltc/types.h>
#include <stddef.h>
#include <stdint.h>

#include "../../../Structs/Color.h"

#define MODEL_ASSET_VERSION 1

/// The maximum number of models that can be loaded in any one execution of the game
#define MAX_MODELS 128

typedef enum ModelShader ModelShader;
typedef enum CollisionModelType CollisionModelType;

typedef struct ModelDefinition ModelDefinition;
typedef struct Material Material;
typedef struct ModelLod ModelLod;
typedef struct ModelConvexHull ModelConvexHull;
typedef struct ModelStaticCollider ModelStaticCollider;

/**
 * List of shaders a model can be rendered with
 */
enum ModelShader
{
	/// The sky shader. Do not use on in-level models.
	SHADER_SKY,
	/// A basic shader with no lighting
	SHADER_UNSHADED,
	/// A shader with basic lighting based on the vertex normals.
	SHADER_SHADED
};

enum CollisionModelType
{
	COLLISION_MODEL_TYPE_NONE,
	COLLISION_MODEL_TYPE_STATIC,
	COLLISION_MODEL_TYPE_DYNAMIC
};

struct Material
{
	/// The texture name of the material
	char *texture;
	/// The tint color of the material
	Color color;
	/// The shader to use for this material
	ModelShader shader;
};

struct ModelLod
{
	/// The runtime-generated ID of this model
	uint32_t id;

	/// How far away the camera must be before this LOD is used (units squared)
	float distanceSquared;

	/// The number of vertices in the model
	size_t vertexCount;
	/// Packed vertex data, (X Y Z) (U V) (R G B A) (NX NY NZ)
	float *vertexData;

	/// The total number of indices across all materials
	uint32_t totalIndexCount;
	/// The number of indices in each material
	uint32_t *indexCount;
	/// Index data for each material
	uint32_t **indexData;
};

struct ModelConvexHull
{
	Vector3 *points;
	Vector3 offset;
	size_t numPoints;
};

struct ModelStaticCollider
{
	size_t numTriangles;
	JPH_Triangle *tris;
};

struct ModelDefinition
{
	/// The runtime-generated ID of this model
	uint32_t id;
	/// The asset name of this model
	char *name;

	/// The number of materials in the model
	uint32_t materialCount;

	uint32_t materialsPerSkin;

	/// The number of skins in the model
	uint32_t skinCount;
	/// The number of LODs in the model
	uint32_t lodCount;

	Material *materials;
	/// The skins for this model, each an array of materialsPerSkin indices into the materials array
	uint32_t **skins;
	/// The LODs for this model
	ModelLod **lods;

	Vector3 boundingBoxOrigin;
	Vector3 boundingBoxExtents;
	JPH_Shape *boundingBoxShape;

	CollisionModelType collisionModelType;


	JPH_Shape *collisionModelShape;

	// union
	// {
	// 	struct
	// 	{
	// 		size_t numHulls;
	// 		ModelConvexHull *hulls;
	// 	};
	// 	ModelStaticCollider staticCollider;
	// };
};

void InitModelLoader();

ModelDefinition *LoadModelInternal(const char *asset);

/**
 * Load a model from an asset
 * @param asset The asset to load the model from
 * @return The loaded model, or NULL if it failed
 */
ModelDefinition *LoadModel(const char *asset);

/**
 * Fetch a cached model from an ID
 * @param id The model ID to fetch
 * @return The model with the given ID
 */
extern ModelDefinition *GetModelFromId(size_t id);

void FreeModel(ModelDefinition *model);

void DestroyModelLoader();

JPH_Shape *CreateDynamicModelShape(size_t numHulls, const ModelConvexHull *hulls);

JPH_Shape *CreateStaticModelShape(const ModelStaticCollider *staticCollider);

#endif //MODELLOADER_H
