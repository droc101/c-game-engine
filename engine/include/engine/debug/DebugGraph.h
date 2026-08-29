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
	/// Number of data points saved in this graph
	size_t numDataPoints;
	/// Pointer to data points
	double *data;

	/// The last time this graph was updated, in milliseconds
	uint64_t lastUpdateTimeMsec;
	/// How often this graph should update, in milliseconds
	uint64_t updateIntervalMsec;

	/// The threshold for a "good" value, anything above shown in green
	double goodThreshold;
	/// The threshold for a "bad" value, anything below shown in red
	double badThreshold;
	/// The maximum value displayed on the graph without being cut off
	double maxShownValue;
	/// Nanoseconds per second/goodThreshold
	double nspf;

	/// Label, shown as "[label]: [value]"
	char *label;
	/// Linear label, shown as "[value] [label]"
	char *linearLabel;
};

/**
 * Create a debug graph
 * @param numDataPoints The number of data points the graph should store
 * @param updateIntervalMsec How often the graph should update, in milliseconds
 * @param goodThreshold The threshold for a "good" value, anything above shown in green
 * @param badThreshold The threshold for a "bad" value, anything below shown in red
 * @param maxShownValue The maximum value displayed on the graph without being cut off
 * @param label Label, shown as "[label]: [value]"
 * @param linearLabel Linear label, shown as "[value] [label]"
 */
DebugGraph *CreateDebugGraph(size_t numDataPoints,
							 uint64_t updateIntervalMsec,
							 double goodThreshold,
							 double badThreshold,
							 double maxShownValue,
							 char *label,
							 char *linearLabel);

/**
 * Destroy a debug graph
 */
void DestroyDebugGraph(DebugGraph *graph);

/**
 * Push a value into a debug graph if it needs to be updated
 * @param graph The graph to push into
 * @param ns The value to push
 */
void DebugGraphPush(DebugGraph *graph, uint64_t ns);

/**
 * Draw a debug graph
 * @param graph The graph to draw
 * @param pos The position to draw at
 * @param size The screen size of the graph
 */
void DrawDebugGraph(DebugGraph *graph, Vector2 pos, Vector2 size);

double DebugGraphGetValue(DebugGraph *graph);
double DebugGraphGetLinearValue(DebugGraph *graph);
double DebugGraphGetRawValue(DebugGraph *graph);

#endif //GAME_DEBUGGRAPH_H
