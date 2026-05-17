//
// Created by droc101 on 10/27/24.
//

#ifndef OPTIONSSTATE_H
#define OPTIONSSTATE_H

#include <engine/structs/GameState.h>
#include <stdbool.h>

extern bool optionsStateInGame;
extern const GameState OptionsState;

void OptionsStateFixedUpdate(GlobalState *state, double delta);

#endif //OPTIONSSTATE_H
