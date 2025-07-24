//
// Created by NBT22 on 7/23/25.
//

#ifndef JOLTDEBUGRENDERER_H
#define JOLTDEBUGRENDERER_H

#include <joltc.h>

void JoltDebugRendererInit();

void JoltDebugRendererDestroy();

void JoltDebugRendererDrawBodies(JPH_PhysicsSystem *physicsSystem);

#endif //JOLTDEBUGRENDERER_H
