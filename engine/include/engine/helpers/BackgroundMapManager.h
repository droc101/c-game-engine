//
// Created by droc101 on 4/7/26.
//

#ifndef GAME_BACKGROUNDMAPMANAGER_H
#define GAME_BACKGROUNDMAPMANAGER_H

#include <engine/structs/GlobalState.h>
#include <stdbool.h>

bool IsBackgroundMapLoaded();

void EnterMenuBackgroundState();

void UpdateMenuBackground(GlobalState *state);

void RenderMenuBackground(GlobalState *state);

void FixedUpdateMenuBackground(GlobalState *state, double delta);

#endif //GAME_BACKGROUNDMAPMANAGER_H
