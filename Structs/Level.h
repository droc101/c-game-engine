//
// Created by droc101 on 4/21/2024.
//

#ifndef GAME_LEVEL_H
#define GAME_LEVEL_H

#include "../defines.h"

/**
 * Create a default empty level
 * @return Blank level
 */
Level *CreateLevel();

/**
 * Destroy a level and everything in it
 * @param l Level to destroy
 */
void DestroyLevel(Level *l);

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
void NameActor(Actor *actor, const char *name, Level *l);

/**
 * Get a single actor by name
 * @param name The name of the actor
 * @param l The level to search in
 * @return The actor with the given name, or NULL if not found
 * @note This is slow. Use sparingly.
 * @note If there are multiple actors with the same name, whichever one was loaded first will be returned
 */
Actor *GetActorByName(const char *name, const Level *l);

/**
 * Get all actors with a given name
 * @param name The name of the actors
 * @param l The level to search in
 * @param actors The list of actors with the given name.
 * @note This is extra slow. Use even more sparingly.
 */
void GetActorsByName(const char *name, const Level *l, List *actors);

#endif //GAME_LEVEL_H
