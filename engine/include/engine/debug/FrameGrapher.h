//
// Created by droc101 on 4/24/24.
//

#ifndef GAME_FRAMEGRAPHER_H
#define GAME_FRAMEGRAPHER_H

#include <stdint.h>

void InitFrameGrapher();

void DestroyFrameGrapher();

void ProcessFrameGrapher();

/**
 * Update the frame graph with the time it took to render the frame
 * @param ns nanoseconds the frame took
 */
void FrameGraphUpdate(uint64_t ns);

/**
 * Draw the frame graph
 */
void FrameGraphDraw();

/**
 * Update the tick graph with the time it took to perform a physics tick
 * @param ns nanoseconds the tick took
 */
void TickGraphUpdate(uint64_t ns);

/**
 * Draw the tick graph
 */
void TickGraphDraw();

#endif //GAME_FRAMEGRAPHER_H
