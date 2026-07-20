//
// Created by droc101 on 4/24/24.
//

#include <engine/debug/DebugGraph.h>
#include <engine/debug/FrameGrapher.h>
#include <engine/graphics/RenderingHelpers.h>
#include <engine/structs/Vector2.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>

DebugGraph *fpsGraph = NULL;
DebugGraph *tpsGraph = NULL;

void InitFrameGrapher()
{
	fpsGraph = CreateDebugGraph(60, 100, 60, 30, 90, "FPS", "MSPF");
	tpsGraph = CreateDebugGraph(60, 100, 60, 50, 60, "TPS", "MSPT");
}

void DestroyFrameGrapher()
{
	DestroyDebugGraph(fpsGraph);
	DestroyDebugGraph(tpsGraph);
}


void FrameGraphUpdate(const uint64_t ns)
{
	DebugGraphPush(fpsGraph, ns);
}

void TickGraphUpdate(const uint64_t ns)
{
	DebugGraphPush(tpsGraph, ns);
}

void FrameGraphDraw()
{
	DrawDebugGraph(fpsGraph, v2(4, ScaledWindowHeight() - 250 - 4), v2(400, 250));
}

void TickGraphDraw()
{
	DrawDebugGraph(tpsGraph, v2(ScaledWindowWidth() - 400 - 4, ScaledWindowHeight() - 250 - 4), v2(400, 250));
}
