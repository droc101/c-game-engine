//
// Created by droc101 on 12/12/24.
//

/**
 * @file PhysicsThread.h
 * Physics thread sync and management
 */

#ifndef PHYSICSTHREAD_H
#define PHYSICSTHREAD_H

#include <engine/structs/GameState.h>
#include <SDL3/SDL_events.h>

/**
 * Start the physics thread
 * @private
 */
void PhysicsThreadInit();

/**
 * Set the function to run in the physics thread
 * @param function The function to run
 * @note This will block until the current iteration of the physics thread is finished
 */
void PhysicsThreadSetFunction(GameStateFixedUpdateFunction function);

/**
 * Process an input event for the physics thread
 * @param event The event to process
 * @private
 */
void PhysicsThreadQueueInputEvent(const SDL_Event *event);

/**
 * Post a quit message to the physics thread and wait for it to finish
 * @private
 */
void PhysicsThreadTerminate();

/**
 * Lock the physics thread mutex
 */
void PhysicsThreadLockTickMutex();

/**
 * Unlock the physics thread mutex
 */
void PhysicsThreadUnlockTickMutex();

/**
 * Get the current physics interpolation factor
 */
float PhysicsInterpolationFactor();

#endif //PHYSICSTHREAD_H
