//
// Created by droc101 on 12/28/24.
//

#include <engine/debug/FrameBenchmark.h>
#include <engine/subsystem/Logging.h>
#include <engine/subsystem/Timing.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>

bool benchRunning = false;
uint64_t benchStartTime;
uint64_t benchFrameCount;
uint64_t highestFrameNs;
uint64_t lowestFrameNs;

uint64_t benchFrameStartTime;

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
#ifdef BENCHMARK_RECORD_HIGH_LOW_TIMES
	if (benchRunning)
	{
		benchFrameStartTime = GetTimeNs();
	}
#endif
}

void BenchFrameEnd()
{
	if (benchRunning)
	{
#ifdef BENCHMARK_RECORD_HIGH_LOW_TIMES
		const uint64_t frameTime = GetTimeNs() - benchFrameStartTime;
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
}

void BenchFinish()
{
	benchRunning = false;
	const uint64_t endTime = GetTimeNs();
	const uint64_t totalTime = endTime - benchStartTime;
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
