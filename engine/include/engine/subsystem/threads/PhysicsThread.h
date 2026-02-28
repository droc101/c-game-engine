//
// Created by droc101 on 12/12/24.
//

#ifndef PHYSICSTHREAD_H
#define PHYSICSTHREAD_H

#include <engine/structs/GlobalState.h>
#include <SDL3/SDL_events.h>

/**
 * Start the physics thread
 */
void PhysicsThreadInit();

/**
 * Set the function to run in the physics thread
 * @param function The function to run
 * @note This will block until the current iteration of the physics thread is finished
 */
void PhysicsThreadSetFunction(FixedUpdateFunction function);

/**
 * Process an input event for the physics thread
 * @param event The event to process
 */
void PhysicsThreadQueueInputEvent(const SDL_Event *event);

/**
 * Post a quit message to the physics thread and wait for it to finish
 */
void PhysicsThreadTerminate();

void PhysicsThreadLockTickMutex();

void PhysicsThreadUnlockTickMutex();

#endif //PHYSICSTHREAD_H
