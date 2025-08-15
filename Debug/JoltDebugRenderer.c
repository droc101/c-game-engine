//
// Created by NBT22 on 7/23/25.
//

#include "JoltDebugRenderer.h"
#include <joltc/joltc.h>

// TODO: Currently only Vulkan is supported, and only the DrawTriangle calls are supported.
//  Ideally this should be fixed, with having GL support the debug renderer alongside Vulkan, as well as implementing at
//  least the DrawLine function if not also the DrawText function. Due to the lack of line drawing, this system cannot
//  currently draw wireframes, and that REALLY should be fixed, either by simply implementing the DrawLine function or
//  through the shader.

#ifdef JPH_DEBUG_RENDERER
#include <stdbool.h>
#include <stddef.h>
#include "../Helpers/Core/Physics/Physics.h"
#include "../Helpers/Graphics/Drawing.h"

static inline bool ShouldDrawBody(void * /*userData*/, const JPH_Body *body)
{
	return JPH_Body_GetObjectLayer(body) != OBJECT_LAYER_PLAYER;
}

static JPH_DebugRenderer *debugRenderer;
static JPH_BodyDrawFilter *drawFilter;
static const JPH_DebugRenderer_Impl debugRendererImpl = {
	.DrawLine = DrawJoltDebugRendererDrawLine,
	.DrawTriangle = DrawJoltDebugRendererDrawTriangle,
};
static const JPH_BodyDrawFilter_Impl bodyDrawFilterImpl = {
	.ShouldDraw = ShouldDrawBody,
};
static const JPH_DrawSettings drawSettings = {
	.drawShape = true,
#ifdef JPH_DEBUG_RENDERER_WIREFRAME
	.drawShapeWireframe = true,
#endif
};

void JoltDebugRendererInit()
{
	debugRenderer = JPH_DebugRenderer_Create(NULL);
	JPH_DebugRenderer_SetImpl(&debugRendererImpl);
	drawFilter = JPH_BodyDrawFilter_Create(NULL);
	JPH_BodyDrawFilter_SetImpl(&bodyDrawFilterImpl);
}

void JoltDebugRendererDestroy()
{
	JPH_DebugRenderer_Destroy(debugRenderer);
	JPH_BodyDrawFilter_Destroy(drawFilter);
}

void JoltDebugRendererDrawBodies(const JPH_PhysicsSystem *physicsSystem)
{
	JPH_PhysicsSystem_DrawBodies(physicsSystem, &drawSettings, debugRenderer, drawFilter);
}
#else
void JoltDebugRendererInit() {}

void JoltDebugRendererDestroy() {}

void JoltDebugRendererDrawBodies(const JPH_PhysicsSystem * /*physicsSystem*/) {}
#endif
