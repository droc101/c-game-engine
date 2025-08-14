//
// Created by droc101 on 10/2/24.
//

#include "RenderingHelpers.h"
#include <cglm/mat4.h>
#include <cglm/types.h>
#include <joltc.h>
#include <SDL_video.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <vulkan/vulkan_core.h>
#include "../../../config.h"
#include "../../Structs/Actor.h"
#include "../../Structs/Color.h"
#include "../../Structs/GlobalState.h"
#include "../../Structs/Level.h"
#include "../../Structs/Vector2.h"
#include "../Core/AssetLoaders/TextureLoader.h"
#include "../Core/Error.h"
#include "../Core/Logging.h"
#include "../Core/MathEx.h"
#include "GL/GLHelper.h"
#include "Vulkan/Vulkan.h"

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

inline int WindowWidth()
{
	return windowWidth;
}

inline int WindowHeight()
{
	return windowHeight;
}

inline float WindowWidthFloat()
{
	return (float)windowWidth;
}

inline float WindowHeightFloat()
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
	int w;
	int h;
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
		JPH_RMatrix4x4 matrix;
		JPH_BodyInterface_GetCenterOfMassTransform(actor->bodyInterface, actor->bodyId, &matrix);
		memcpy(*transformMatrix, &matrix, sizeof(mat4));
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
			GL_Swap();
			break;
		default:
			break;
	}
}

void LoadLevelWalls(const Level *l)
{
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:
			VK_LoadLevelWalls(l);
			break;
		case RENDERER_OPENGL:
			GL_LoadLevelWalls(l);
			break;
		default:
			break;
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

inline void GetColor(const uint32_t argb, Color *color)
{
	color->r = (float)(argb >> 16 & 0xFF) / 255.0f;
	color->g = (float)(argb >> 8 & 0xFF) / 255.0f;
	color->b = (float)(argb >> 0 & 0xFF) / 255.0f;
	color->a = (float)(argb >> 24 & 0xFF) / 255.0f;
}
