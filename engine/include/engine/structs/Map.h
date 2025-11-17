//
// Created by droc101 on 4/21/2024.
//

#ifndef GAME_LEVEL_H
#define GAME_LEVEL_H

#include <joltc/joltc.h>
#include <stdbool.h>
#include <stdint.h>
#include <engine/structs/List.h>
#include <engine/structs/Player.h>
#include <engine/structs/Actor.h>
#include <engine/structs/Camera.h>

typedef struct Map Map;

struct Map
{
	/// The list of actors in the level
	LockingList actors;

	/// The fully qualified texture name of the sky
	char skyTexture[80];

	/// The color of the fog
	uint32_t fogColor;
	/// The distance from the player at which the fog begins to fade in
	float fogStart;
	/// The distance from the player at which the fog is fully opaque
	float fogEnd;

	JPH_PhysicsSystem *physicsSystem;
	uint32_t floorBodyId;

	/// The player object
	Player player;

	/// The map of named actors in the level (key portion)
	LockingList namedActorNames;
	/// The map of named actors in the level (value portion)
	List namedActorPointers;

	/// A pointer to the I/O proxy actor, if it exists
	Actor *ioProxy;
};

/**
 * Create a default empty level
 * @return Blank level
 */
Map *CreateLevel(void);

/**
 * Destroy a level and everything in it
 * @param level Level to destroy
 */
void DestroyLevel(Map *level);

/**
 * Add an actor to the level
 * @param actor Actor to add
 * @note This is intended to be used during gameplay, not level loading
 */
void AddActor(Actor *actor);

/**
 * Remove an actor from the level
 * @param actor Actor to remove
 * @note This is intended to be used during gameplay, not level loading
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
 * @param l The level to search in
 * @return The actor with the given name, or NULL if not found
 * @note This is slow. Use sparingly.
 * @note If there are multiple actors with the same name, whichever one was loaded first will be returned
 */
Actor *GetActorByName(const char *name, const Map *l);

/**
 * Get all actors with a given name
 * @param name The name of the actors
 * @param l The level to search in
 * @param actors The list of actors with the given name.
 * @note This is extra slow. Use even more sparingly.
 */
void GetActorsByName(const char *name, const Map *l, List *actors);

/**
 * Renders a level from a given camera, including actor UI and physics debug.
 * @param level The level to render
 * @param camera The camera to view from
 */
void RenderLevel(const Map *level, const Camera *camera);

#endif //GAME_LEVEL_H
