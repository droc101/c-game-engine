//
// Created by NBT22 on 7/23/25.
//

#include "JoltDebugRenderer.h"

#include "../Helpers/Graphics/Drawing.h"

static JPH_DebugRenderer *debugRenderer;
static JPH_BodyDrawFilter *drawFilter;
static const JPH_DebugRenderer_Procs debugRendererProcs = {
	.DrawTriangle = DrawJoltDebugRendererTriangle,
};
static const JPH_DrawSettings drawSettings = {
	.drawShape = true,
};

void JoltDebugRendererInit()
{
	debugRenderer = JPH_DebugRenderer_Create(NULL);
	JPH_DebugRenderer_SetProcs(&debugRendererProcs);
	drawFilter = JPH_BodyDrawFilter_Create(NULL);
}

void JoltDebugRendererDrawBodies(JPH_PhysicsSystem *physicsSystem)
{
	JPH_PhysicsSystem_DrawBodies(physicsSystem, &drawSettings, debugRenderer, drawFilter);
}
