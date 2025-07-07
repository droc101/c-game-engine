//
// Created by droc101 on 4/24/24.
//

#ifndef GAME_FRAMEGRAPHER_H
#define GAME_FRAMEGRAPHER_H

#include "../defines.h"

/// How many frames to store in the graph
#define FRAMEGRAPH_HISTORY_SIZE 30

/// How often to update the graph (in MS)
#define FRAMEGRAPH_INTERVAL 100

// Thresholds for coloring the graph
/// Anything above this is good (green)
#define FRAMEGRAPH_THRESHOLD_GOOD 60
#define TICKGRAPH_THRESHOLD_GOOD 60
/// Anything below this is bad (red)
#define FRAMEGRAPH_THRESHOLD_BAD 30
#define TICKGRAPH_THRESHOLD_BAD 30
// Anything between these two are orange

/// Enable or disable capping the graph at 2x the target FPS
#define FRAMEGRAPH_ENABLE_CAPPING

// How much to scale the graph by (60fps target and 2x scale makes it a nice square)
#define FRAMEGRAPH_V_SCALE 2
#define FRAMEGRAPH_H_SCALE 8

#define TICKGRAPH_V_SCALE 2
#define TICKGRAPH_H_SCALE 8

/// Enable or disable the 2nd graph line for linear time
#define FRAMEGRAPH_SHOW_LINEAR_TIME_GRAPH

/// Disable to draw the graph
/// Drawing the graph has a significant performance impact
//#define FRAMEGRAPH_FPS_ONLY

/// no touchy please 🥺
#define FRAMEGRAPH_NSPF (1000000000.0 / FRAMEGRAPH_THRESHOLD_GOOD)

/**
 * Update the frame graph with the time it took to render the frame
 * @param ns nanoseconds the frame took
 */
void FrameGraphUpdate(ulong ns);

/**
 * Draw the frame graph
 */
void FrameGraphDraw();

void TickGraphUpdate(const ulong ns);

void TickGraphDraw();

#endif //GAME_FRAMEGRAPHER_H
