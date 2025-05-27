//
// Created by droc101 on 4/21/2024.
//

#include "Drawing.h"
#include <stdio.h>
#include "../../defines.h"
#include "../../Structs/GlobalState.h"
#include "../../Structs/Vector2.h"
#include "../Core/AssetReader.h"
#include "../Core/Error.h"
#include "../Core/Logging.h"
#include "GL/GLHelper.h"
#include "RenderingHelpers.h"
#include "SDL.h"
#include "Vulkan/Vulkan.h"
#include "../../Structs/Level.h"


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
							  const Vector2 region_start,
							  const Vector2 region_end)
{
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:
			VK_DrawTexturedQuadRegion((int)pos.x,
									  (int)pos.y,
									  (int)size.x,
									  (int)size.y,
									  (int)region_start.x,
									  (int)region_start.y,
									  (int)region_end.x,
									  (int)region_end.y,
									  texture);
			break;
		case RENDERER_OPENGL:
			GL_DrawTextureRegion(pos, size, texture, region_start, region_end);
			break;
		default:
			break;
	}
}

inline void DrawTextureRegionMod(const Vector2 pos,
								 const Vector2 size,
								 const char *texture,
								 const Vector2 region_start,
								 const Vector2 region_end,
								 const Color color)
{
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:
			VK_DrawTexturedQuadRegionMod((int)pos.x,
										 (int)pos.y,
										 (int)size.x,
										 (int)size.y,
										 (int)region_start.x,
										 (int)region_start.y,
										 (int)region_end.x,
										 (int)region_end.y,
										 texture,
										 color);
			break;
		case RENDERER_OPENGL:
			GL_DrawTextureRegionMod(pos, size, texture, region_start, region_end, color);
			break;
		default:
			break;
	}
}

inline void ClearColor(const Color color)
{
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:
			break;
		case RENDERER_OPENGL:
			GL_ClearColor(color);
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
						  const float output_margins_px,
						  const float texture_margins_px,
						  const char *texture)
{
	const Vector2 textureSize = GetTextureSize(texture);

	DrawTextureRegion(pos, v2s(output_margins_px), texture, v2s(0), v2s(texture_margins_px)); // top left
	DrawTextureRegion(v2(pos.x, pos.y + output_margins_px),
					  v2(output_margins_px, size.y - texture_margins_px * 2),
					  texture,
					  v2(0, texture_margins_px),
					  v2(texture_margins_px, textureSize.y - texture_margins_px * 2)); // middle left
	DrawTextureRegion(v2(pos.x, pos.y + size.y - output_margins_px),
					  v2s(output_margins_px),
					  texture,
					  v2(0, textureSize.y - texture_margins_px),
					  v2s(texture_margins_px)); // bottom left

	DrawTextureRegion(v2(pos.x + output_margins_px, pos.y),
					  v2(size.x - texture_margins_px * 2, output_margins_px),
					  texture,
					  v2(texture_margins_px, 0),
					  v2(textureSize.x - texture_margins_px * 2, texture_margins_px)); // top middle
	DrawTextureRegion(v2(pos.x + output_margins_px, pos.y + output_margins_px),
					  v2(size.x - texture_margins_px * 2, size.y - texture_margins_px * 2),
					  texture,
					  v2(texture_margins_px, texture_margins_px),
					  v2(textureSize.x - texture_margins_px * 2,
						 textureSize.y - texture_margins_px * 2)); // middle middle
	DrawTextureRegion(v2(pos.x + output_margins_px, pos.y + (size.y - output_margins_px)),
					  v2(size.x - texture_margins_px * 2, output_margins_px),
					  texture,
					  v2(texture_margins_px, textureSize.y - texture_margins_px),
					  v2(textureSize.x - texture_margins_px * 2, texture_margins_px)); // bottom middle

	DrawTextureRegion(v2(pos.x + (size.x - output_margins_px), pos.y),
					  v2s(output_margins_px),
					  texture,
					  v2(textureSize.x - texture_margins_px, 0),
					  v2s(texture_margins_px)); // top right
	DrawTextureRegion(v2(pos.x + (size.x - output_margins_px), pos.y + output_margins_px),
					  v2(output_margins_px, size.y - texture_margins_px * 2),
					  texture,
					  v2(textureSize.x - texture_margins_px, texture_margins_px),
					  v2(texture_margins_px, textureSize.y - texture_margins_px * 2)); // middle right
	DrawTextureRegion(v2(pos.x + (size.x - output_margins_px), pos.y + (size.y - output_margins_px)),
					  v2s(output_margins_px),
					  texture,
					  v2(textureSize.x - texture_margins_px, textureSize.y - texture_margins_px),
					  v2s(texture_margins_px)); // bottom right
}

inline void DrawBatchedQuadsTextured(const BatchedQuadArray *batch, const char *texture, const Color color)
{
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:
			VK_DrawTexturedQuadsBatched(batch->verts, batch->quad_count, texture, color);
		break;
		case RENDERER_OPENGL:
			GL_DrawTexturedArrays(batch->verts, batch->indices, batch->quad_count, texture, color);
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
			VK_DrawColoredQuadsBatched(batch->verts, batch->quad_count, color);
		break;
		case RENDERER_OPENGL:
			GL_DrawColoredArrays(batch->verts, batch->indices, batch->quad_count, color);
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
