//
// Created by droc101 on 11/20/25.
//

#include <cglm/cglm.h>
#include <engine/graphics/gl/GLdebug.h>
#include <engine/graphics/gl/GLframe.h>
#include <engine/graphics/gl/GLobjects.h>
#include <engine/graphics/RenderingHelpers.h>
#include <SDL_video.h>
#include <stdbool.h>
#include <stddef.h>

bool GL_FrameStart()
{
	GL_ResetDebugLines();
	return true;
}

inline void GL_ClearScreen()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

inline void GL_ClearDepthOnly()
{
	glClear(GL_DEPTH_BUFFER_BIT);
}

inline void GL_Swap()
{
	SDL_GL_SwapWindow(GetGameWindow());
}

inline void GL_UpdateViewportSize()
{
	int vpWidth = 0;
	int vpHeight = 0;
	SDL_GL_GetDrawableSize(GetGameWindow(), &vpWidth, &vpHeight);
	glViewport(0, 0, vpWidth, vpHeight);
}
