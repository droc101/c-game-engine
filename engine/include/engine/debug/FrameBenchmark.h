//
// Created by droc101 on 12/28/24.
//

#ifndef FRAMEBENCHMARK_H
#define FRAMEBENCHMARK_H

// Enable or disable the frame benchmark system
#define BENCHMARK_SYSTEM_ENABLE

// Enable or disable recording highest and lowest frame times (has a performance impact)
// #define BENCHMARK_RECORD_HIGH_LOW_TIMES

/**
 * Record the start time of the frame for benchmarking
 */
void BenchFrameStart();

/**
 * Record the end time of the frame for benchmarking and update the highest and lowest frame times
 */
void BenchFrameEnd();

/**
 * Start or stop the benchmarking system
 */
void BenchToggle();

#endif //FRAMEBENCHMARK_H
