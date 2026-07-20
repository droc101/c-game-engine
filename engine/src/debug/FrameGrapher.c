//
// Created by droc101 on 4/24/24.
//

#include <engine/debug/DebugGraph.h>
#include <engine/debug/FrameGrapher.h>
#include <engine/graphics/RenderingHelpers.h>
#include <engine/structs/Vector2.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "engine/subsystem/Input.h"

static DebugGraph *fpsGraph = NULL;
static DebugGraph *tpsGraph = NULL;

typedef enum
{
	FRAME_GRAPHER_HIDDEN,
	FRAME_GRAPHER_TEXT,
	FRAME_GRAPHER_FULL,
	FRAME_GRAPHER_MAX
} FrameGrapherMode;

static FrameGrapherMode mode = FRAME_GRAPHER_TEXT;

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

void ProcessFrameGrapher()
{
	if (IsKeyJustPressed(mainThreadInput, SDL_SCANCODE_F4))
	{
		mode++;
		if (mode == FRAME_GRAPHER_MAX)
		{
			mode = 0;
		}
	}
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
	if (mode == FRAME_GRAPHER_FULL)
	{
		DrawDebugGraph(fpsGraph, v2(4, ScaledWindowHeight() - 250 - 4), v2(400, 250));
	} else if (mode == FRAME_GRAPHER_TEXT)
	{
		DrawDebugGraphText(fpsGraph, v2(4, ScaledWindowHeight() - 250 - 4), v2(400, 250), false);
	}
}

void TickGraphDraw()
{
	if (mode == FRAME_GRAPHER_FULL)
	{
		DrawDebugGraph(tpsGraph, v2(ScaledWindowWidth() - 400 - 4, ScaledWindowHeight() - 250 - 4), v2(400, 250));
	} else if (mode == FRAME_GRAPHER_TEXT)
	{
		DrawDebugGraphText(tpsGraph,
						   v2(ScaledWindowWidth() - 400 - 4, ScaledWindowHeight() - 250 - 4),
						   v2(400, 250),
						   true);
	}
}
