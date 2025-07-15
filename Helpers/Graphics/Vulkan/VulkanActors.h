//
// Created by NBT22 on 5/8/25.
//

#ifndef VULKANACTORS_H
#define VULKANACTORS_H

#include <vulkan/vulkan.h>
#include "../../../defines.h"

void LoadLod(const Actor *actor, uint32_t lodIndex);

VkResult InitActors(const List *actors);

VkResult LoadActorWalls(const List *actors);

VkResult UpdateActorInstanceData(const List *actors);

void DestroyActorMetadata();

#endif //VULKANACTORS_H
