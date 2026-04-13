//
// Created by droc101 on 4/7/26.
//

#ifndef GAME_MAPPHYSICS_H
#define GAME_MAPPHYSICS_H

#include <engine/structs/GlobalState.h>

void MapUpdate(GlobalState *state, double delta);

void MapFixedUpdate(GlobalState *state, double delta);

#endif //GAME_MAPPHYSICS_H
