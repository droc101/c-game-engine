//
// Created by droc101 on 10/27/24.
//

#ifndef OPTIONSSTATE_H
#define OPTIONSSTATE_H

#define GAME_STATE_OPTIONS 9

#include <stdbool.h>

extern bool optionsStateInGame;

void OptionsStateSet(bool inGame);

void OptionsStateDestroy();

#endif //OPTIONSSTATE_H
