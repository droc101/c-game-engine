//
// Created by droc101 on 10/26/26.
//

#ifndef GAME_LOCK_H
#define GAME_LOCK_H

#include <stdbool.h>

/**
 * Create a lock preventing multiple instances from running
 * @return success/failure, where failure indicates another instance is already running
 */
bool CreateMultiInstanceLock();

/**
 * Destroy the lock preventing multiple instances from running
 */
void DestroyMultiInstanceLock();

#endif //GAME_LOCK_H
