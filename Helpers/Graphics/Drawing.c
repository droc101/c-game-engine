//
// Created by droc101 on 4/21/2024.
//

#include "Drawing.h"
#include <stdio.h>
#include "../../defines.h"
#include "../../Structs/GlobalState.h"
#include "../../Structs/Level.h"
#include "../../Structs/Vector2.h"
#include "../Core/AssetReader.h"
#include "../Core/Error.h"
#include "../Core/Logging.h"
#include "GL/GLHelper.h"
#include "RenderingHelpers.h"
#include "SDL.h"
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

inline void DrawTextureMod(const Vector2 pos, const Vector2 size, const char *texture, const Color color)
{
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:
			VK_DrawTexturedQuadMod((int)pos.x, (int)pos.y, (int)size.x, (int)size.y, texture, color);
			break;
		case RENDERER_OPENGL:
			GL_DrawTextureMod(pos, size, texture, color);
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

	DrawTextureRegion(pos, v2s(outputMarginsPx), texture, v2s(0), v2s(textureMarginsPx)); // top left
	DrawTextureRegion(v2(pos.x, pos.y + outputMarginsPx),
					  v2(outputMarginsPx, size.y - textureMarginsPx * 2),
					  texture,
					  v2(0, textureMarginsPx),
					  v2(textureMarginsPx, textureSize.y - textureMarginsPx * 2)); // middle left
	DrawTextureRegion(v2(pos.x, pos.y + size.y - outputMarginsPx),
					  v2s(outputMarginsPx),
					  texture,
					  v2(0, textureSize.y - textureMarginsPx),
					  v2s(textureMarginsPx)); // bottom left

	DrawTextureRegion(v2(pos.x + outputMarginsPx, pos.y),
					  v2(size.x - textureMarginsPx * 2, outputMarginsPx),
					  texture,
					  v2(textureMarginsPx, 0),
					  v2(textureSize.x - textureMarginsPx * 2, textureMarginsPx)); // top middle
	DrawTextureRegion(v2(pos.x + outputMarginsPx, pos.y + outputMarginsPx),
					  v2(size.x - textureMarginsPx * 2, size.y - textureMarginsPx * 2),
					  texture,
					  v2(textureMarginsPx, textureMarginsPx),
					  v2(textureSize.x - textureMarginsPx * 2,
						 textureSize.y - textureMarginsPx * 2)); // middle middle
	DrawTextureRegion(v2(pos.x + outputMarginsPx, pos.y + (size.y - outputMarginsPx)),
					  v2(size.x - textureMarginsPx * 2, outputMarginsPx),
					  texture,
					  v2(textureMarginsPx, textureSize.y - textureMarginsPx),
					  v2(textureSize.x - textureMarginsPx * 2, textureMarginsPx)); // bottom middle

	DrawTextureRegion(v2(pos.x + (size.x - outputMarginsPx), pos.y),
					  v2s(outputMarginsPx),
					  texture,
					  v2(textureSize.x - textureMarginsPx, 0),
					  v2s(textureMarginsPx)); // top right
	DrawTextureRegion(v2(pos.x + (size.x - outputMarginsPx), pos.y + outputMarginsPx),
					  v2(outputMarginsPx, size.y - textureMarginsPx * 2),
					  texture,
					  v2(textureSize.x - textureMarginsPx, textureMarginsPx),
					  v2(textureMarginsPx, textureSize.y - textureMarginsPx * 2)); // middle right
	DrawTextureRegion(v2(pos.x + (size.x - outputMarginsPx), pos.y + (size.y - outputMarginsPx)),
					  v2s(outputMarginsPx),
					  texture,
					  v2(textureSize.x - textureMarginsPx, textureSize.y - textureMarginsPx),
					  v2s(textureMarginsPx)); // bottom right
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

void RenderMenuBackground()
{
	// sorry for the confusing variable names
	const Vector2 bgTileSize = v2(320, 240); // size on screen
	const Vector2 bgTexSize = GetTextureSize(TEXTURE("interface_menu_bg_tile")); // actual size of the texture

	const Vector2 tilesOnScreen = v2(WindowWidthFloat() / bgTileSize.x, WindowHeightFloat() / bgTileSize.y);
	const Vector2 tileRegion = v2(tilesOnScreen.x * bgTexSize.x, tilesOnScreen.y * bgTexSize.y);
	DrawTextureRegion(v2(0, 0),
					  v2(WindowWidthFloat(), WindowHeightFloat()),
					  TEXTURE("interface_menu_bg_tile"),
					  v2(0, 0),
					  tileRegion);
}

void RenderInGameMenuBackground()
{
	RenderLevel(GetState());

	DrawRect(0, 0, WindowWidth(), WindowHeight(), COLOR(0xA0000000));
}

void RenderLevel3D(const Level *l, const Camera *cam)
{
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:
			VK_RenderLevel(l, cam);
			break;
		case RENDERER_OPENGL:
			GL_RenderLevel(l, cam);
			break;
		default:
			break;
	}
}
