//
// Created by NBT22 on 5/8/25.
//

#ifndef VULKANACTORS_H
#define VULKANACTORS_H

#include <vulkan/vulkan.h>
#include "../../../defines.h"

VkResult InitActors(const Level *level);

void LoadModelActors(const Level *level);

void LoadActorDrawInfo(const Level *level);

VkResult LoadWallActors(const Level *level);

#endif //VULKANACTORS_H
