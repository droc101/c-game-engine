//
// Created by droc101 on 4/21/2024.
//

#ifndef GAME_MAP_H
#define GAME_MAP_H

#include <engine/assets/MapMaterialLoader.h>
#include <engine/structs/Actor.h>
#include <engine/structs/Camera.h>
#include <engine/structs/List.h>
#include <engine/structs/Player.h>
#include <joltc/joltc.h>
#include <joltc/Math/Vector3.h>
#include <stdbool.h>
#include <stdint.h>
#include "engine/structs/Color.h"
#include "engine/structs/Vector2.h"

typedef struct Map Map;
typedef struct MapVertex MapVertex;
typedef struct MapModel MapModel;

struct MapVertex
{
	Vector3 position;
	Vector2 uv;
	Color color;
	Vector3 normal;
};

struct MapModel
{
	MapMaterial *material;
	uint32_t numVerts;
	MapVertex *verts;
	uint32_t numIndices;
	uint32_t *indices;
};

struct Map
{
	char *discordRpcIcon;
	char *discordRpcName;

	/// The list of actors in the map
	LockingList actors;

	size_t numModels;
	MapModel *models;

	List joltBodies;

	/// The fully qualified texture name of the sky
	char *skyTexture;

	/// The color of the fog
	uint32_t fogColor;
	/// The distance from the player at which the fog begins to fade in
	float fogStart;
	/// The distance from the player at which the fog is fully opaque
	float fogEnd;

	JPH_PhysicsSystem *physicsSystem;

	/// The player object
	Player player;

	/// The map of named actors in the map (key portion)
	LockingList namedActorNames;
	/// The map of named actors in the map (value portion)
	List namedActorPointers;

	/// A pointer to the I/O proxy actor, if it exists
	Actor *ioProxy;
};

/**
 * Create a default empty map
 * @return Blank map
 */
Map *CreateMap(void);

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
 * @param l
 */
void NameActor(Actor *actor, const char *name, Map *l);

/**
 * Get a single actor by name
 * @param name The name of the actor
 * @param mapo The map to search in
 * @return The actor with the given name, or NULL if not found
 * @note This is slow. Use sparingly.
 * @note If there are multiple actors with the same name, whichever one was loaded first will be returned
 */
Actor *GetActorByName(const char *name, const Map *mapo);

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
