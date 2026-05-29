//
// Created by Noah on 12/18/2024.
//

#ifndef VULKANRESOURCES_H
#define VULKANRESOURCES_H

#include <engine/assets/TextureLoader.h>
#include <stdbool.h>

VkResult ResizeDebugDrawBuffers();

bool LoadTexture(const Image *image);

#endif //VULKANRESOURCES_H
