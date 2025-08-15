//
// Created by NBT22 on 7/23/25.
//

#ifndef JOLTDEBUGRENDERER_H
#define JOLTDEBUGRENDERER_H

#include <joltc/joltc.h>

/**
 * Set up the static variables required for the Jolt debug renderer
 */
void JoltDebugRendererInit();

/**
 * Destroy the static variables created by the @c JoltDebugRendererInit function
 */
void JoltDebugRendererDestroy();

/**
 * Draw the bodies of a particular physics system using Jolt's built-in debug rendering system
 * @param physicsSystem The Jolt physics system to draw the bodies of
 */
void JoltDebugRendererDrawBodies(const JPH_PhysicsSystem *physicsSystem);

#endif //JOLTDEBUGRENDERER_H
