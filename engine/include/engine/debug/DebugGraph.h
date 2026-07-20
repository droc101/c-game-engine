//
// Created by droc101 on 7/19/26.
//

#ifndef GAME_DEBUGGRAPH_H
#define GAME_DEBUGGRAPH_H

#include <engine/structs/Vector2.h>
#include <stddef.h>
#include <stdint.h>

typedef struct DebugGraph DebugGraph;

struct DebugGraph
{
	size_t numDataPoints;
	double *data;

	uint64_t lastUpdateTimeMsec;
	uint64_t updateIntervalMsec;

	double goodThreshold;
	double badThreshold;
	double maxShownValue;
	double nspf;

	char *label;
	char *linearLabel;
};

DebugGraph *CreateDebugGraph(size_t numDataPoints,
							 uint64_t updateIntervalMsec,
							 double goodThreshold,
							 double badThreshold,
							 double maxShownValue,
							 char *label,
							 char *linearLabel);

void DestroyDebugGraph(DebugGraph *graph);

void DebugGraphPush(DebugGraph *graph, uint64_t ns);

void DrawDebugGraph(DebugGraph *graph, Vector2 pos, Vector2 size);

double DebugGraphGetValue(DebugGraph *graph);
double DebugGraphGetLinearValue(DebugGraph *graph);
double DebugGraphGetRawValue(DebugGraph *graph);

#endif //GAME_DEBUGGRAPH_H
