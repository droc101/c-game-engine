//
// Created by droc101 on 4/21/2024.
//

#include "Drawing.h"
#include <joltc/enums.h>
#include <joltc/Math/RVec3.h>
#include <joltc/Math/Vector3.h>
#include <joltc/types.h>
#include <SDL_error.h>
#include <SDL_hints.h>
#include <SDL_surface.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "../../Structs/Camera.h"
#include "../../Structs/Color.h"
#include "../../Structs/GlobalState.h"
#include "../../Structs/Level.h"
#include "../../Structs/Vector2.h"
#include "../Core/AssetLoaders/TextureLoader.h"
#include "../Core/AssetReader.h"
#include "../Core/Error.h"
#include "../Core/Logging.h"
#include "../Core/Physics/Player.h"
#include "Font.h"
#include "GL/GLHelper.h"
#include "RenderingHelpers.h"
#include "Vulkan/Vulkan.h"

SDL_Surface *ToSDLSurface(const char *texture, const char *filterMode)
{
	const Image *img = LoadImage(texture);

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, filterMode);

	SDL_Surface *surface = SDL_CreateRGBSurfaceFrom(img->pixelData,
													(int)img->width,
													(int)img->height,
													32,
													(int)img->width * 4,
													0x00ff0000,
													0x0000ff00,
													0x000000ff,
													0xff000000);
	if (surface == NULL)
	{
		LogError("Failed to create surface: %s\n", SDL_GetError());
		Error("Failed to create surface");
	}

	return surface;
}

// Rendering subsystem abstractions

inline void DrawLine(const Vector2 start, const Vector2 end, const float thickness, const Color color)
{
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:
			VK_DrawLine((int)start.x,
						(int)start.y,
						(int)end.x,
						(int)end.y,
						(int)(thickness * GetState()->uiScale),
						color);
			break;
		case RENDERER_OPENGL:
			GL_DrawLine(start, end, color, (float)(thickness * GetState()->uiScale));
			break;
		default:
			break;
	}
}

inline void DrawOutlineRect(const Vector2 pos, const Vector2 size, const float thickness, const Color color)
{
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:
			VK_DrawRectOutline((int)pos.x,
							   (int)pos.y,
							   (int)size.x,
							   (int)size.y,
							   (int)(thickness * GetState()->uiScale),
							   color);
			break;
		case RENDERER_OPENGL:
			GL_DrawRectOutline(pos, size, color, (float)(thickness * GetState()->uiScale));
			break;
		default:
			break;
	}
}

inline void DrawTexture(const Vector2 pos, const Vector2 size, const char *texture)
{
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:
			VK_DrawTexturedQuad((int)pos.x, (int)pos.y, (int)size.x, (int)size.y, texture);
			break;
		case RENDERER_OPENGL:
			GL_DrawTexture(pos, size, texture);
			break;
		default:
			break;
	}
}

inline void DrawTextureMod(const Vector2 pos, const Vector2 size, const char *texture, const Color *color)
{
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:
			VK_DrawTexturedQuadMod((int)pos.x, (int)pos.y, (int)size.x, (int)size.y, texture, color);
			break;
		case RENDERER_OPENGL:
			GL_DrawTextureMod(pos, size, texture, *color);
			break;
		default:
			break;
	}
}

inline void DrawTextureRegion(const Vector2 pos,
							  const Vector2 size,
							  const char *texture,
							  const Vector2 regionStart,
							  const Vector2 regionEnd)
{
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:
			VK_DrawTexturedQuadRegion((int)pos.x,
									  (int)pos.y,
									  (int)size.x,
									  (int)size.y,
									  (int)regionStart.x,
									  (int)regionStart.y,
									  (int)regionEnd.x,
									  (int)regionEnd.y,
									  texture);
			break;
		case RENDERER_OPENGL:
			GL_DrawTextureRegion(pos, size, texture, regionStart, regionEnd);
			break;
		default:
			break;
	}
}

inline void DrawTextureRegionMod(const Vector2 pos,
								 const Vector2 size,
								 const char *texture,
								 const Vector2 regionStart,
								 const Vector2 regionEnd,
								 const Color color)
{
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:
			VK_DrawTexturedQuadRegionMod((int)pos.x,
										 (int)pos.y,
										 (int)size.x,
										 (int)size.y,
										 (int)regionStart.x,
										 (int)regionStart.y,
										 (int)regionEnd.x,
										 (int)regionEnd.y,
										 texture,
										 color);
			break;
		case RENDERER_OPENGL:
			GL_DrawTextureRegionMod(pos, size, texture, regionStart, regionEnd, color);
			break;
		default:
			break;
	}
}

inline void ClearScreen()
{
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:
			// Unused
			break;
		case RENDERER_OPENGL:
			GL_ClearScreen();
			break;
		default:
			break;
	}
}

inline void ClearDepthOnly()
{
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:
			// Unused
			break;
		case RENDERER_OPENGL:
			GL_ClearDepthOnly();
			break;
		default:
			break;
	}
}

inline void DrawRect(const int x, const int y, const int w, const int h, const Color color)
{
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:
			VK_DrawColoredQuad(x, y, w, h, color);
			break;
		case RENDERER_OPENGL:
			GL_DrawRect(v2((float)x, (float)y), v2((float)w, (float)h), color);
			break;
		default:
			break;
	}
}

void DrawNinePatchTexture(const Vector2 pos,
						  const Vector2 size,
						  const float outputMarginsPx,
						  const float textureMarginsPx,
						  const char *texture)
{
	const Vector2 textureSize = GetTextureSize(texture);
	const Vector2 marginUvSize = v2((1.0f / textureSize.x) * textureMarginsPx,
									(1.0f / textureSize.y) * textureMarginsPx);

	float vertices[16][4] = {
		{pos.x, pos.y, 0.0f, 0.0f},
		{pos.x + outputMarginsPx, pos.y, marginUvSize.x, 0.0f},
		{pos.x + size.x - outputMarginsPx, pos.y, 1.0f - marginUvSize.x, 0.0f},
		{pos.x + size.x, pos.y, 1.0f, 0.0f},

		{pos.x, pos.y + outputMarginsPx, 0.0f, marginUvSize.y},
		{pos.x + outputMarginsPx, pos.y + outputMarginsPx, marginUvSize.x, marginUvSize.y},
		{pos.x + size.x - outputMarginsPx, pos.y + outputMarginsPx, 1.0f - marginUvSize.x, marginUvSize.y},
		{pos.x + size.x, pos.y + outputMarginsPx, 1.0f, marginUvSize.y},

		{pos.x, pos.y + size.y - outputMarginsPx, 0.0f, 1.0f - marginUvSize.y},
		{pos.x + outputMarginsPx, pos.y + size.y - outputMarginsPx, marginUvSize.x, 1.0f - marginUvSize.y},
		{pos.x + size.x - outputMarginsPx,
		 pos.y + size.y - outputMarginsPx,
		 1.0f - marginUvSize.x,
		 1.0f - marginUvSize.y},
		{pos.x + size.x, pos.y + size.y - outputMarginsPx, 1.0f, 1.0f - marginUvSize.y},

		{pos.x, pos.y + size.y, 0.0f, 1.0f},
		{pos.x + outputMarginsPx, pos.y + size.y, marginUvSize.x, 1.0f},
		{pos.x + size.x - outputMarginsPx, pos.y + size.y, 1.0f - marginUvSize.x, 1.0f},
		{pos.x + size.x, pos.y + size.y, 1.0f, 1.0f},
	};

	uint32_t indices[2 * 9][3] = {
		{4, 1, 0},
		{1, 4, 5},

		{5, 2, 1},
		{5, 6, 2},

		{6, 3, 2},
		{3, 6, 7},

		{8, 5, 4},
		{5, 8, 9},

		{9, 6, 5},
		{6, 9, 10},

		{10, 7, 6},
		{7, 10, 11},

		{12, 9, 8},
		{9, 12, 13},

		{13, 10, 9},
		{10, 13, 14},

		{14, 11, 10},
		{11, 14, 15},
	};

	for (int i = 0; i < 16; i++)
	{
		vertices[i][0] = X_TO_NDC(vertices[i][0]);
		vertices[i][1] = Y_TO_NDC(vertices[i][1]);
	}

	const UiTriangleArray tris = {
		.vertices = vertices,
		.vertexCount = sizeof(vertices) / sizeof(**vertices),
		.indices = indices,
		.indexCount = sizeof(indices) / sizeof(**indices),
	};

	DrawUiTriangles(&tris, texture, COLOR_WHITE);
}

inline void DrawBatchedQuadsTextured(const BatchedQuadArray *batch, const char *texture, const Color color)
{
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:
			VK_DrawTexturedQuadsBatched(batch->verts, batch->quadCount, texture, color);
			break;
		case RENDERER_OPENGL:
			GL_DrawTexturedArrays(batch->verts, batch->indices, batch->quadCount, texture, color);
			break;
		default:
			break;
	}
}

inline void DrawBatchedQuadsColored(const BatchedQuadArray *batch, const Color color)
{
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:
			VK_DrawColoredQuadsBatched(batch->verts, batch->quadCount, color);
			break;
		case RENDERER_OPENGL:
			GL_DrawColoredArrays(batch->verts, batch->indices, batch->quadCount, color);
			break;
		default:
			break;
	}
}

inline void DrawUiTriangles(const UiTriangleArray *triangleArray, const char *texture, const Color color)
{
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:
			VK_DrawUiTriangles(triangleArray, texture, color);
			break;
		case RENDERER_OPENGL:
			GL_DrawUITriangles(triangleArray, texture, color);
			break;
		default:
			LogWarning("DrawUiTriangles not implemented in current renderer!\n");
	}
}

void DrawJoltDebugRendererDrawLine(void * /*userData*/,
								   const JPH_RVec3 *from,
								   const JPH_RVec3 *to,
								   const JPH_Color color)
{
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:
			VK_DrawJoltDebugRendererLine(from, to, color);
			break;
		default:
		case RENDERER_OPENGL:
			break;
	}
}

void DrawJoltDebugRendererDrawTriangle(void * /*userData*/,
									   const JPH_RVec3 *v1,
									   const JPH_RVec3 *v2,
									   const JPH_RVec3 *v3,
									   const JPH_Color color,
									   JPH_DebugRenderer_CastShadow /*castShadow*/)
{
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:
			VK_DrawJoltDebugRendererTriangle((Vector3[]){*v1, *v2, *v3}, color);
			break;
		default:
		case RENDERER_OPENGL:
			break;
	}
}

void RenderMenuBackground()
{
	// sorry for the confusing variable names
	const Vector2 bgTileSize = v2(320, 240); // size on screen
	const Vector2 bgTexSize = GetTextureSize(TEXTURE("interface/menu_bg_tile")); // actual size of the texture

	const Vector2 tilesOnScreen = v2(WindowWidthFloat() / bgTileSize.x, WindowHeightFloat() / bgTileSize.y);
	const Vector2 tileRegion = v2(tilesOnScreen.x * bgTexSize.x, tilesOnScreen.y * bgTexSize.y);
	DrawTextureRegion(v2(0, 0),
					  v2(WindowWidthFloat(), WindowHeightFloat()),
					  TEXTURE("interface/menu_bg_tile"),
					  v2(0, 0),
					  tileRegion);
}

void RenderInGameMenuBackground()
{
	const GlobalState *state = GetState();
	RenderLevel(state->level, state->camera);
	RenderHUD();
	DrawRect(0, 0, WindowWidth(), WindowHeight(), COLOR(0xA0000000));
}

void RenderHUD()
{
	const GlobalState *state = GetState();
	Vector2 coinIconPos = v2(WindowWidth() - 260, 16);
	const Vector2 coinIconSize = v2s(40);
	DrawTexture(v2(WindowWidthFloat() - 260, 16), coinIconSize, TEXTURE("interface/hud_ycoin"));

	char coinStr[16];
	sprintf(coinStr, "%d", state->saveData->coins);
	FontDrawString(v2(WindowWidthFloat() - 210, 16), coinStr, 40, COLOR_WHITE, largeFont);

	coinIconPos.y = 64;

	for (int blueCoinIndex = 0; blueCoinIndex < state->saveData->blueCoins; blueCoinIndex++)
	{
		coinIconPos.x = (float)WindowWidth() - 260 + (float)blueCoinIndex * 48;
		DrawTexture(coinIconPos, coinIconSize, TEXTURE("interface/hud_bcoin"));
	}

	const Color *crosshairColor = GetCrosshairColor();

	DrawTextureMod(v2((WindowWidth() * 0.5) - 12, (WindowHeight() * 0.5) - 12),
				   v2s(24),
				   TEXTURE("interface/crosshair"),
				   crosshairColor);
}

void RenderLevel3D(const Level *l, const Camera *cam)
{
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:
			VK_RenderLevel(l, cam, &GetState()->viewmodel);
			break;
		case RENDERER_OPENGL:
			GL_RenderLevel(l, cam);
			break;
		default:
			break;
	}
}
