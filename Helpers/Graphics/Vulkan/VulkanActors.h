//
// Created by NBT22 on 5/8/25.
//

#ifndef VULKANACTORS_H
#define VULKANACTORS_H

#include <vulkan/vulkan.h>
#include "../../../Helpers/Core/List.h"

void VulkanActorsVariablesInit();

void VulkanActorsVariablesCleanup();

VkResult InitActors(const LockingList *actors);

#endif //VULKANACTORS_H
