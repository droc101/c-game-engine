//
// Created by droc101 on 4/24/24.
//

#ifndef GAME_FRAMEGRAPHER_H
#define GAME_FRAMEGRAPHER_H

#include <stdint.h>
#include <engine/debug/DebugGraph.h>

/// The debug FPS graph
extern DebugGraph *fpsGraph;
/// The debug TPS graph
extern DebugGraph *tpsGraph;

/**
 * Initialize the frame grapher
 * @private
 */
void InitFrameGrapher();

/**
 * Destroy the frame grapher
 * @private
 */
void DestroyFrameGrapher();

/**
 * Update the frame graph with the time it took to render the frame
 * @param ns nanoseconds the frame took
 * @private
 */
void FrameGraphUpdate(uint64_t ns);

/**
 * Draw the frame graph
 * @private
 */
void FrameGraphDraw();

/**
 * Update the tick graph with the time it took to perform a physics tick
 * @param ns nanoseconds the tick took
 * @private
 */
void TickGraphUpdate(uint64_t ns);

/**
 * Draw the tick graph
 * @private
 */
void TickGraphDraw();

#endif //GAME_FRAMEGRAPHER_H
