//
// Created by droc101 on 10/2/24.
//

#include <cglm/affine.h>
#include <cglm/mat4.h>
#include <cglm/types.h>
#include <engine/assets/TextureLoader.h>
#include <engine/graphics/gl/GLframe.h>
#include <engine/graphics/gl/GLinit.h>
#include <engine/graphics/gl/GLworld.h>
#include <engine/graphics/RenderingHelpers.h>
#include <engine/graphics/vulkan/Vulkan.h>
#include <engine/helpers/MathEx.h>
#include <engine/structs/Actor.h>
#include <engine/structs/Color.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/Map.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Logging.h>
#include <joltc/constants.h>
#include <joltc/Math/RMat44.h>
#include <joltc/Physics/Body/BodyInterface.h>
#include <SDL3/SDL_video.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <vulkan/vulkan_core.h>

Renderer currentRenderer;
bool lowFPSMode;

SDL_Window *window;
int windowWidth;
int windowHeight;

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

inline Vector2 ActualWindowSizeIgnoreDPI()
{
	int w = 0;
	int h = 0;
	SDL_GetWindowSizeInPixels(window, &w, &h);
	return v2((float)w, (float)h);
}

inline Vector2 ActualWindowSize()
{
	int w = 0;
	int h = 0;
	SDL_GetWindowSize(window, &w, &h);
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
		if (actor->actorModel != NULL &&
			(actor->actorFlags & ACTOR_FLAG_USING_BOUNDING_BOX_COLLISION) == ACTOR_FLAG_USING_BOUNDING_BOX_COLLISION)
		{
			glm_translate(*transformMatrix,
						  (vec3){-actor->actorModel->boundingBoxOrigin.x,
								 -actor->actorModel->boundingBoxOrigin.y,
								 -actor->actorModel->boundingBoxOrigin.z});
		}
	} else
	{
		LogWarning("ActorTransformMatrix called on actor which has no body!\n");
		glm_mat4_identity(*transformMatrix);
	}
}

bool RenderPreInit()
{
	currentRenderer = GetState()->options.renderer;
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:
			return true;
		case RENDERER_OPENGL:
			return GL_PreInit();
		default:
			return false;
	}
}

bool RenderInit()
{
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:
			return VK_Init(GetGameWindow());
		case RENDERER_OPENGL:
			return GL_Init(GetGameWindow());
		default:
			return false;
	}
}

void RenderDestroy()
{
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:
			VK_Cleanup();
			break;
		case RENDERER_OPENGL:
			GL_DestroyGL();
			break;
		default:
			break;
	}
}

bool FrameStart()
{
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:
			return VK_FrameStart() == VK_SUCCESS;
		case RENDERER_OPENGL:
			return GL_FrameStart();
		default:
			return true;
	}
}

void FrameEnd()
{
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:
			VK_FrameEnd();
			break;
		case RENDERER_OPENGL:
			GL_FrameEnd();
			break;
		default:
			break;
	}
	GetState()->map->changeFlags = 0;
}

inline void UpdateViewportSize()
{
	const float newScaleX = ActualWindowSize().x / DEF_WIDTH;
	const float newScaleY = ActualWindowSize().y / DEF_HEIGHT;
	float newScale = newScaleX < newScaleY ? newScaleX : newScaleY;
	newScale = max(newScale, 1.0f);
	GetState()->uiScale = newScale;
	UpdateWindowSize();
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:
			// Unused
			break;
		case RENDERER_OPENGL:
			GL_UpdateViewportSize();
			break;
		default:
			break;
	}
}

inline void WindowObscured()
{
	lowFPSMode = true;
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:
			VK_Minimize();
			break;
		case RENDERER_OPENGL:
		default:
			break;
	}
}

inline void WindowRestored()
{
	lowFPSMode = false;
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:
			VK_Restore();
			break;
		case RENDERER_OPENGL:
		default:
			break;
	}
}

inline void SetLowFPS(const bool val)
{
	lowFPSMode = val;
}

inline bool IsLowFPSModeEnabled()
{
	return lowFPSMode && GetState()->options.limitFpsWhenUnfocused;
}

inline float X_TO_NDC(const float x)
{
	switch (currentRenderer)
	{
		case RENDERER_VULKAN: // NOLINT(*-branch-clone)
			return VK_X_TO_NDC(x);
		case RENDERER_OPENGL:
			return GL_X_TO_NDC(x);
		default:
			return 0;
	}
}

inline float Y_TO_NDC(const float y)
{
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:
			return VK_Y_TO_NDC(y);
		case RENDERER_OPENGL:
			return GL_Y_TO_NDC(y);
		default:
			return 0;
	}
}

void LoadMapModels(const Map *map)
{
	switch (currentRenderer)
	{
		case RENDERER_OPENGL:
			GL_LoadMap(map);
			break;
		case RENDERER_VULKAN:
			// TODO
		default:
			break;
	}
}

inline void GetColor(const uint32_t argb, Color *color)
{
	color->r = (float)(argb >> 16 & 0xFF) / 255.0f;
	color->g = (float)(argb >> 8 & 0xFF) / 255.0f;
	color->b = (float)(argb >> 0 & 0xFF) / 255.0f;
	color->a = (float)(argb >> 24 & 0xFF) / 255.0f;
}
