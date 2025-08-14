//
// Created by droc101 on 4/26/2024.
//

#include "Timing.h"
#include <bits/time.h>
#include <stdint.h>
#include <time.h>

static uint64_t StartTimeNS;
static uint64_t StartTimeS;

void InitTimers()
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	StartTimeNS = ts.tv_sec * 1000000000 + ts.tv_nsec;
	StartTimeS = ts.tv_sec;
}

uint64_t GetTimeNs()
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec * 1000000000 + ts.tv_nsec - StartTimeNS;
}

uint64_t GetTimeMs()
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000) - (StartTimeNS / 1000000);
}

uint64_t GetTimeS()
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec - StartTimeS;
}
