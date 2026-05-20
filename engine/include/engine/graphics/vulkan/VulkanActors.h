//
// Created by NBT22 on 2/9/26.
//

#ifndef GAME_VULKANACTORS_H
#define GAME_VULKANACTORS_H

#include <engine/structs/List.h>
#include <vulkan/vulkan_core.h>

void InitActorLoadingVariables();

VkResult LoadActors(const LockingList *actors);

VkResult UpdateActors();

#endif //GAME_VULKANACTORS_H
