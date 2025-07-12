//
// Created by NBT22 on 5/8/25.
//

#ifndef VULKANACTORS_H
#define VULKANACTORS_H

#include <vulkan/vulkan.h>
#include "../../../defines.h"

VkResult InitActors(const List *actors);

void LoadActorModels(const List *actors);

void LoadActorDrawInfo(const List *actors);

VkResult LoadActorWalls(const List *actors);

VkResult UpdateActorInstanceDataAndShadows(const List *actors);

#endif //VULKANACTORS_H
