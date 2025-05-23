//
// Created by droc101 on 12/28/24.
//

#include "FrameBenchmark.h"

#include <limits.h>

#include "../defines.h"
#include "../Helpers/Core/Logging.h"
#include "../Helpers/Core/Timing.h"

bool benchRunning = false;
ulong benchStartTime;
ulong benchFrameCount;
ulong highestFrameNs;
ulong lowestFrameNs;

ulong benchFrameStartTime;

void BenchStart()
{
	benchRunning = true;
	benchStartTime = GetTimeNs();
	benchFrameCount = 0;
	highestFrameNs = 0;
	lowestFrameNs = ULONG_MAX;
	BenchFrameStart();
	LogInfo("Benchmark started\n");
}

void BenchFrameStart()
{
	if (!benchRunning)
	{
		return;
	}
#ifdef BENCHMARK_RECORD_HIGH_LOW_TIMES
	benchFrameStartTime = GetTimeNs();
#endif
}

void BenchFrameEnd()
{
	if (!benchRunning)
	{
		return;
	}
#ifdef BENCHMARK_RECORD_HIGH_LOW_TIMES
	const ulong frameTime = GetTimeNs() - benchFrameStartTime;
	if (frameTime < lowestFrameNs)
	{
		lowestFrameNs = frameTime;
	}
	if (frameTime > highestFrameNs)
	{
		highestFrameNs = frameTime;
	}
#endif
	benchFrameCount++;
}

void BenchFinish()
{
	benchRunning = false;
	const ulong endTime = GetTimeNs();
	const ulong totalTime = endTime - benchStartTime;
	double avgFrameTime = (double)totalTime / (double)benchFrameCount;
	const double avgFps = 1.0 / (avgFrameTime / 1000000000.0);

	avgFrameTime /= 1000000.0;

	LogInfo("Benchmark finished\n");
	LogInfo("Average frame time: %f ms\n", avgFrameTime);
	LogInfo("Average FPS: %f\n", avgFps);

#ifdef BENCHMARK_RECORD_HIGH_LOW_TIMES
	const double lowestFrameTime = (double)lowestFrameNs / 1000000.0;
	const double highestFrameTime = (double)highestFrameNs / 1000000.0;

	LogInfo("Lowest frame time: %f ms\n", lowestFrameTime);
	LogInfo("Highest frame time: %f ms\n", highestFrameTime);
#endif
}

void BenchToggle()
{
	if (benchRunning)
	{
		BenchFinish();
	} else
	{
		BenchStart();
	}
}
