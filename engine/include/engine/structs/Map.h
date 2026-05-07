//
// Created by droc101 on 4/21/2024.
//

#ifndef GAME_MAP_H
#define GAME_MAP_H

#include <engine/assets/MapMaterialLoader.h>
#include <engine/structs/Actor.h>
#include <engine/structs/Camera.h>
#include <engine/structs/Color.h>
#include <engine/structs/Light.h>
#include <engine/structs/List.h>
#include <engine/structs/Player.h>
#include <engine/structs/Vector2.h>
#include <engine/structs/Viewmodel.h>
#include <joltc/joltc.h>
#include <joltc/Math/Vector3.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct Map Map;
typedef struct MapVertex MapVertex;
typedef struct MapModel MapModel;

typedef enum MapChangeFlags MapChangeFlags;

enum MapChangeFlags
{
	MAP_LIGHT_CHANGED = 1 << 1,
	MAP_FOG_CHANGED = 1 << 2,
	MAP_VIEWMODEL_CHANGED = 1 << 3
};

struct MapVertex
{
	/// The world space position of the vertex
	Vector3 position;
	/// The UV coordinate
	Vector2 uv;
	/// The Lightmap UV coordiante
	Vector2 lightmapUv;
};

struct MapModel
{
	/// The material this model uses
	MapMaterial *material;
	/// The number of vertices in this model
	uint32_t vertexCount;
	/// The vertices in this model
	MapVertex *vertices;
	/// The number of indices in this model
	uint32_t indexCount;
	/// The indices in this model
	uint32_t *indices;
};

struct Map
{
	char *mapName;
	/// The name of the icon this map uses for Discord RPC
	char *discordRpcIcon;
	/// The display name this map uses for Discord RPC
	char *discordRpcName;

	/// The list of actors in the map
	LockingList actors;

	/// Ths number of map models in this map
	size_t modelCount;
	/// The map models
	MapModel *models;

	List joltBodies;

	bool renderSky;
	/// The fully qualified texture name of the sky
	char *skyTexture;

	/// The color of the fog
	Color fogColor;
	/// The distance from the player at which the fog begins to fade in
	float fogStart;
	/// The distance from the player at which the fog is fully opaque
	float fogEnd;

	/// The global light color. The alpha channel is ignored.
	Color lightColor;
	/// HDR tonemapping exposure
	float exposure;

	JPH_PhysicsSystem *physicsSystem;
	uint64_t physicsTick;

	/// The player object
	Player player;

	/// The map of named actors in the map (key portion)
	LockingList namedActorNames;
	/// The map of named actors in the map (value portion)
	List namedActorPointers;

	/// A pointer to the I/O proxy actor, if it exists
	Actor *ioProxy;

	MapChangeFlags changeFlags;

	/// The view model
	Viewmodel viewmodel;

	/// The width of the lightmap in pixels
	size_t lightmapWidth;
	/// The height of the lightmap in pixels
	size_t lightmapHeight;
	/// The lightmap data in RGBA16F format
	void *lightmapPixels;

	/// The number of point lights in the level
	uint16_t numPointLights;
	/// The point lights in this level
	PointLight *pointLights;
};

/**
 * Create a default empty map
 * @return Blank map
 */
Map *CreateMap(void);

/**
 * Free data from a map that is only used when first loading it
 * @param map The map to free data from
 */
void FreeLoadTimeMapData(Map *map);

/**
 * Destroy a map and everything in it
 * @param map Map to destroy
 */
void DestroyMap(Map *map);

/**
 * Add an actor to the map
 * @param actor Actor to add
 * @note This is intended to be used during gameplay, not map loading
 */
void AddActor(Actor *actor);

/**
 * Remove an actor from the map
 * @param actor Actor to remove
 * @note This is intended to be used during gameplay, not map loading
 */
void RemoveActor(Actor *actor);

/**
 * Assign a name to an actor
 * @param actor The actor to name
 * @param name The name to assign
 * @param map The map within which the actor resides
 */
void NameActor(Actor *actor, const char *name, Map *map);

/**
 * Get a single actor by name
 * @param name The name of the actor
 * @param map The map to search in
 * @return The actor with the given name, or NULL if not found
 * @note This is slow. Use sparingly.
 * @note If there are multiple actors with the same name, whichever one was loaded first will be returned
 */
Actor *GetActorByName(const char *name, const Map *map);

/**
 * Get all actors with a given name
 * @param name The name of the actors
 * @param map The map to search in
 * @param actors The list of actors with the given name.
 * @note This is extra slow. Use even more sparingly.
 */
void GetActorsByName(const char *name, const Map *map, List *actors);

/**
 * Renders a map from a given camera, including actor UI and physics debug.
 * @param map The map to render
 * @param camera The camera to view from
 */
void RenderMap(const Map *map, const Camera *camera);

#endif //GAME_MAP_H
