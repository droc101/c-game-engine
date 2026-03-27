//
// Created by droc101 on 4/22/2024.
//

#ifndef MENUSTATE_H
#define MENUSTATE_H

#include <engine/structs/GameState.h>
#include <stdbool.h>

// Shows a level select after main menu and pause exit course.
#define USE_LEVEL_SELECT

extern bool menuStateFadeIn;
extern const GameState MenuState;

#endif //MENUSTATE_H
