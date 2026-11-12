//
// Created by droc101 on 4/26/2024.
//

/**
 * @file Timing.h
 * Monotonic clock system
 */

#ifndef GAME_TIMING_H
#define GAME_TIMING_H

#include <engine/subsystem/Logging.h>
#include <stdint.h>

/**
 * Perform a simple timing benchmark of some code
 * @param thingToTime The code to time
 */
#define TIME(thingToTime) \
	{ \
		const long double startTime = GetTimeNs(); \
		thingToTime; \
		const long double endTime = GetTimeNs(); \
		LogInfo("%Lf ms\n", (endTime - startTime) / 1000000); \
	}

/**
 * Initialize the timers to count from the start of the program
 * @private
 */
void InitTimers();

/**
 * Get the time the program has been running in nanoseconds
 * @return Time in nanoseconds
 * @warning Do not use to track time spent in a map, this increments when the game is paused!
 */
uint64_t GetTimeNs();

/**
 * Get the time the program has been running in milliseconds
 * @return Time in milliseconds
 * @warning Do not use to track time spent in a map, this increments when the game is paused!
 */
uint64_t GetTimeMs();

/**
 * Get the time the program has been running in seconds
 * @return Time in seconds
 * @warning Do not use to track time spent in a map, this increments when the game is paused!
 */
uint64_t GetTimeS();

#endif //GAME_TIMING_H
