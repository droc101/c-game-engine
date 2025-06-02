//
// Created by droc101 on 12/12/24.
//

#ifndef PHYSICSTHREAD_H
#define PHYSICSTHREAD_H

#include "../../defines.h"

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
 * Post a quit message to the physics thread and wait for it to finish
 */
void PhysicsThreadTerminate();

void PhysicsThreadLockTickMutex();

void PhysicsThreadUnlockTickMutex();

#endif //PHYSICSTHREAD_H
