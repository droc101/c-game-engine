//
// Created by droc101 on 4/7/26.
//

#ifndef GAME_MAPPHYSICS_H
#define GAME_MAPPHYSICS_H

#include <engine/structs/GlobalState.h>

/**
 * Perform a frame update on the map
 * @param state GlobalState containing the map
 * @param delta Delta time
 */
void MapUpdate(GlobalState *state, double delta);

/**
 * Perform a physics tick update on the map
 * @param state GlobalState containing the map
 * @param delta Delta time
 */
void MapFixedUpdate(GlobalState *state, double delta);

#endif //GAME_MAPPHYSICS_H
