//
// Created by NBT22 on 2/9/26.
//

#ifndef GAME_VULKANACTORS_H
#define GAME_VULKANACTORS_H

#include <engine/structs/List.h>

void InitActorLoadingVariables();

void ClearActorLodMaterialSlotsData();

void LoadActors(const LockingList *actors);

void UpdateActors();

#endif //GAME_VULKANACTORS_H
