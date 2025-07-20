//
// Created by droc101 on 4/26/2024.
//

#ifndef GAME_TIMING_H
#define GAME_TIMING_H

#include "../../defines.h"
#include "../Core/Logging.h"

#define TIME(thingToTime) \
	{ \
		const long double startTime = GetTimeNs(); \
		thingToTime; \
		const long double endTime = GetTimeNs(); \
		LogInfo("%Lf ms\n", (endTime - startTime) / 1000000); \
	}

/**
 * Initialize the timers to count from the start of the program
 */
void InitTimers();

/**
 * Get the time the program has been running in nanoseconds
 * @return Time in nanoseconds
 */
ulong GetTimeNs();

/**
 * Get the time the program has been running in milliseconds
 * @return Time in milliseconds
 */
ulong GetTimeMs();

/**
 * Get the time the program has been running in seconds
 * @return Time in seconds
 */
ulong GetTimeS();

#endif //GAME_TIMING_H
