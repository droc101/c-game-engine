//
// Created by droc101 on 7/23/25.
//

#ifndef MODELLOADER_H
#define MODELLOADER_H

#include <engine/structs/Color.h>
#include <engine/structs/Vector2.h>
#include <joltc/joltc.h>
#include <joltc/Math/Vector3.h>
#include <joltc/Physics/Collision/Shape/Shape.h>
#include <stddef.h>
#include <stdint.h>

#define MODEL_ASSET_VERSION 1

/// The maximum number of models that can be loaded in any one execution of the game
#define MAX_MODELS 128

typedef enum ModelShader ModelShader;
typedef enum CollisionModelType CollisionModelType;

typedef struct Material Material;
typedef struct ModelVertex ModelVertex;
typedef struct ModelLod ModelLod;
typedef struct ModelDefinition ModelDefinition;
typedef struct ModelConvexHull ModelConvexHull;
typedef struct ModelStaticCollider ModelStaticCollider;

/**
 * List of shaders a model can be rendered with
 */
enum ModelShader
{
	/// The sky shader. Do not use on standard models.
	SHADER_SKY,
	/// A basic shader with no lighting
	SHADER_UNSHADED,
	/// A shader with basic lighting based on the vertex normals.
	SHADER_SHADED
};

enum CollisionModelType
{
	/// This model does not contain a collision model.
	COLLISION_MODEL_TYPE_NONE,
	/// This model contains a static collision mesh (@c JPH_MeshShape)
	COLLISION_MODEL_TYPE_STATIC,
	/// This model contains a set of convex hulls in a compound shape (@c JPH_StaticCompoundShape)
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

struct ModelVertex
{
	/// The position of the vertex, in model space
	Vector3 position;
	/// The texture coordinate of the vertex
	Vector2 uv;
	/// The color of the vertex
	Color color;
	/// The normal of the vertex, as a unit vector
	Vector3 normal;
} __attribute__((packed));

struct ModelLod
{
	/// The runtime-generated ID of this LOD
	uint32_t id;

	/// How far away the camera must be before this LOD is used (units squared)
	float distanceSquared;

	/// The number of vertices in the LOD
	size_t vertexCount;
	/// The vertex data for the LOD
	ModelVertex *vertexData;

	/// The total number of indices across all material slots
	uint32_t totalIndexCount;
	/// The number of indices in each material slot, indexed with a slot number
	uint32_t *indexCount;
	/// Index data for each material slot, indexed with a slot number
	uint32_t **indexData;
};

struct ModelDefinition
{
	/// The runtime-generated ID of this model
	uint32_t id;
	/// The asset name of this model
	char *name;

	/// The total number of materials in the model, across all skins
	uint32_t materialCount;

	/// The number of materials per skin
	uint32_t materialSlotCount;

	/// The number of skins in the model
	uint32_t skinCount;
	/// The number of LODs in the model
	uint32_t lodCount;

	/// An array of materials with length @c materialCount
	Material *materials;
	/// An array of length @c skinCount, where each element is an array of length @c materialSlotCount,
	/// where each element is an index into the @c materials array
	uint32_t **skinMaterialIndices;
	/// The LODs for this model
	ModelLod *lods;

	/// The origin (center) of the bounding box
	Vector3 boundingBoxOrigin;
	/// The extends of the bounding box. The box will be twice this size.
	Vector3 boundingBoxExtents;
	/// The jolt shape for the bounding box
	JPH_Shape *boundingBoxShape;

	/// The type of collision model this model contains
	CollisionModelType collisionModelType;
	/// The jolt collision shape for this model, or @c NULL if there isn't one
	JPH_Shape *collisionModelShape;
};

struct ModelConvexHull
{
	/// The points of the hull
	Vector3 *points;
	/// The offset of the hull
	Vector3 offset;
	/// The number of points in the hull
	size_t numPoints;
};

struct ModelStaticCollider
{
	/// The number of triangles in this mesh
	size_t numTriangles;
	/// The triangles in this mesh
	JPH_Triangle *tris;
};

/**
 * Initialize the model loader and try to load the error model
 */
void InitModelLoader();

/**
 * Internal function to load a model.
 * @param asset The path to load
 * @return The loaded model, or @c NULL on error
 */
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

/**
 * Free a model asset
 * @param model The model to free
 */
void FreeModel(ModelDefinition *model);

/**
 * Destroy the model loader
 */
void DestroyModelLoader();

/**
 * Create a dynamic model collider shape
 * @param numHulls The number of hulls
 * @param hulls The hulls
 * @return The @c JPH_Shape
 */
JPH_Shape *CreateDynamicModelShape(size_t numHulls, const ModelConvexHull *hulls);

/**
 * Create a static model collider shape
 * @param staticCollider The static collider data
 * @return The @c JPH_Shape
 */
JPH_Shape *CreateStaticModelShape(const ModelStaticCollider *staticCollider);

#endif //MODELLOADER_H
