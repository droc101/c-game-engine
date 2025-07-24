//
// Created by NBT22 on 7/23/25.
//

#include "JoltDebugRenderer.h"

#include "../Helpers/Graphics/Drawing.h"

bool ShouldDrawBody(void * /*userData*/, const JPH_Body *body);

static JPH_DebugRenderer *debugRenderer;
static JPH_BodyDrawFilter *drawFilter;
static const JPH_DebugRenderer_Procs debugRendererProcs = {
	.DrawTriangle = DrawJoltDebugRendererTriangle,
};
static const JPH_BodyDrawFilter_Procs bodyDrawFilterProcs = {
	.ShouldDraw = ShouldDrawBody,
};
bool(JPH_API_CALL *ShouldDraw)(void *userData, const JPH_Body *body);
static const JPH_DrawSettings drawSettings = {
	.drawShape = true,
};

bool ShouldDrawBody(void * /*userData*/, const JPH_Body *body)
{
	return false;
	return JPH_Body_GetObjectLayer(body) == OBJECT_LAYER_STATIC;
}

void JoltDebugRendererInit()
{
	debugRenderer = JPH_DebugRenderer_Create(NULL);
	JPH_DebugRenderer_SetProcs(&debugRendererProcs);
	drawFilter = JPH_BodyDrawFilter_Create(NULL);
	JPH_BodyDrawFilter_SetProcs(&bodyDrawFilterProcs);
}

void JoltDebugRendererDestroy()
{
	JPH_DebugRenderer_Destroy(debugRenderer);
	JPH_BodyDrawFilter_Destroy(drawFilter);
}

void JoltDebugRendererDrawBodies(JPH_PhysicsSystem *physicsSystem)
{
	JPH_PhysicsSystem_DrawBodies(physicsSystem, &drawSettings, debugRenderer, drawFilter);
}
