//
// Created by droc101 on 10/27/24.
//

#ifndef GAME_GOPTIONSSTATE_H
#define GAME_GOPTIONSSTATE_H

#include <stdbool.h>

extern bool optionsStateInGame;

void GOptionsStateSet(bool inGame);

void GOptionsStateDestroy();

#endif //GAME_GOPTIONSSTATE_H
