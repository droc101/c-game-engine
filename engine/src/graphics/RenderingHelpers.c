//
// Created by droc101 on 10/2/24.
//

#include <assert.h>
#include <cglm/affine.h>
#include <cglm/mat4.h>
#include <cglm/types.h>
#include <engine/assets/AssetReader.h>
#include <engine/assets/TextureLoader.h>
#include <engine/graphics/RenderingHelpers.h>
#include <engine/graphics/vulkan/Vulkan.h>
#include <engine/helpers/MathEx.h>
#include <engine/structs/Actor.h>
#include <engine/structs/Color.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/Map.h>
#include <engine/structs/Options.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Logging.h>
#include <joltc/Math/RMat44.h>
#include <joltc/Physics/Body/BodyID.h>
#include <joltc/Physics/Body/BodyInterface.h>
#include <SDL3/SDL_video.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <vulkan/vulkan_core.h>

bool windowFocused;

SDL_Window *window;
int windowWidth;
int windowHeight;

RendererQueuedAction rendererQueuedActions = 0;
OptionsMsaa qaNewFrameufferMsaaValue = MSAA_NONE;

void SetGameWindow(SDL_Window *w)
{
	window = w;
}

inline SDL_Window *GetGameWindow()
{
	return window;
}

inline int ScaledWindowWidth()
{
	return windowWidth;
}

inline int ScaledWindowHeight()
{
	return windowHeight;
}

inline float ScaledWindowWidthFloat()
{
	return (float)windowWidth;
}

inline float ScaledWindowHeightFloat()
{
	return (float)windowHeight;
}

inline void UpdateWindowSize()
{
	SDL_GetWindowSize(window, &windowWidth, &windowHeight);
	windowWidth = (int)(windowWidth / GetState()->uiScale);
	windowHeight = (int)(windowHeight / GetState()->uiScale);
}

inline Vector2 ActualWindowSize()
{
	int w = 0;
	int h = 0;
	SDL_GetWindowSize(window, &w, &h);
	return v2((float)w, (float)h);
}

inline Vector2 ActualWindowSizeIgnoreDPI()
{
	int w = 0;
	int h = 0;
	SDL_GetWindowSizeInPixels(window, &w, &h);
	return v2((float)w, (float)h);
}

Vector2 GetTextureSize(const char *texture)
{
	const Image *img = LoadImage(texture);

	return v2((float)img->width, (float)img->height);
}

void ActorTransformMatrix(const Actor *actor, mat4 *transformMatrix)
{
	if (!transformMatrix)
	{
		Error("A NULL transformMatrix must not be passed to ActorTransformMatrix!");
	}
	if (actor->bodyId != JPH_BodyId_InvalidBodyID && actor->bodyInterface != NULL)
	{
		JPH_RMat44 matrix;
		JPH_BodyInterface_GetWorldTransform(actor->bodyInterface, actor->bodyId, &matrix);
		memcpy(*transformMatrix, &matrix, sizeof(mat4));
		if (actor->model != NULL &&
			(actor->flags & ACTOR_FLAG_USING_BOUNDING_BOX_COLLISION) == ACTOR_FLAG_USING_BOUNDING_BOX_COLLISION)
		{
			glm_translate(*transformMatrix,
						  (vec3){-actor->model->boundingBoxOrigin.x,
								 -actor->model->boundingBoxOrigin.y,
								 -actor->model->boundingBoxOrigin.z});
		}
	} else
	{
		LogWarning("ActorTransformMatrix called on actor which has no body!\n");
		glm_mat4_identity(*transformMatrix);
	}
}

bool RenderPreInit()
{
	return VK_PreInit();
}

bool RenderInit()
{
	return VK_Init(GetGameWindow());
}

void RenderDestroy()
{
	VK_Cleanup();
}

bool FrameStart()
{
	if ((rendererQueuedActions & QUEUED_ACTION_RELOAD_ALL_ASSETS) != 0)
	{
		HotReloadAssets();
		rendererQueuedActions &= ~QUEUED_ACTION_RELOAD_ALL_ASSETS;
	}
	return VK_FrameStart();
}

void FrameEnd()
{
	VK_FrameEnd();
	if (GetState()->map)
	{
		GetState()->map->changeFlags = 0;
	}
}

inline void UpdateViewportSize()
{
	const float newScaleX = ActualWindowSize().x / DEF_WIDTH;
	const float newScaleY = ActualWindowSize().y / DEF_HEIGHT;
	float newScale = newScaleX < newScaleY ? newScaleX : newScaleY;
	newScale = max(newScale, 1.0f);
	GetState()->uiScale = newScale;
	UpdateWindowSize();
	VK_UpdateViewportSize();
}

inline void WindowObscured()
{
	windowFocused = false;
	VK_Minimize();
}

inline void WindowRestored()
{
	VK_Restore();
}

inline void SetWindowFocused(const bool val)
{
	windowFocused = val;
}

inline bool IsWindowFocused()
{
	return windowFocused;
}

inline bool IsLowFPSModeEnabled()
{
	return !windowFocused && GetState()->options.limitFpsWhenUnfocused;
}

inline float X_TO_NDC(const float x)
{
	return VK_X_TO_NDC(x);
}

inline float Y_TO_NDC(const float y)
{
	return VK_Y_TO_NDC(y);
}

void LoadMapModels(Map *map)
{
	assert(map->lightmapPixels && map->models);
	VK_LoadMap(map);
	FreeLoadTimeMapData(map);
}

void SetVsyncEnabled(const bool enabled)
{
	// TODO
}

inline void GetColor(const uint32_t argb, Color *color)
{
	color->r = (float)(argb >> 16 & 0xFF) / 255.0f;
	color->g = (float)(argb >> 8 & 0xFF) / 255.0f;
	color->b = (float)(argb >> 0 & 0xFF) / 255.0f;
	color->a = (float)(argb >> 24 & 0xFF) / 255.0f;
}
