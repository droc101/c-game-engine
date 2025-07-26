//
// Created by NBT22 on 7/23/25.
//

#include "JoltDebugRenderer.h"

#include "../Helpers/Graphics/Drawing.h"

/*TODO: There should be a lot more configuration for how this works.
 * The whole system should only exist if we have enabled the system in Jolt (with the cmake option).
 *  If this is not the case, then this file will be referencing functions which are not implemented.
 * Currently the way that disabling the system works is by just always returning false in `ShouldDrawBody`, but this
 *  has many issues associated with it. The first issue is that the calls are still being made, just then being
 *  discarded by the filter, leading to decreased performance even when not drawing the debug overlay. Additionally
 *  there is the fact that all of the framework to draw the overlay is still being created even if it isn't going to be
 *  used. While this isn't a huge issue (one unused pipeline and <100k of VRAM with no new allocations), it still should
 *  be addressed for a full release type of situation.
 * Currently only Vulkan is supported, and only the DrawTriangle calls are supported. Ideally this should be fixed, with
 *	having GL support the debug renderer alongside Vulkan, as well as implementing at least the DrawLine function if not
 *	also the DrawText function. Due to the lack of line drawing, this system cannot currently draw wireframes, and that
 *	REALLY should be fixed, either by simply implementing the DrawLine function or through the shader.
 */

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
	return JPH_Body_GetObjectLayer(body) != OBJECT_LAYER_PLAYER;
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

void JoltDebugRendererDrawBodies(const JPH_PhysicsSystem *physicsSystem)
{
	JPH_PhysicsSystem_DrawBodies(physicsSystem, &drawSettings, debugRenderer, drawFilter);
}
