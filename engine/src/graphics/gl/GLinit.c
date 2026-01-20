//
// Created by droc101 on 9/30/2024.
//

#include <engine/graphics/gl/GLdebug.h>
#include <engine/graphics/gl/GLframe.h>
#include <engine/graphics/gl/GLinit.h>
#include <engine/graphics/gl/GLobjects.h>
#include <engine/graphics/gl/GLshaders.h>
#include <engine/graphics/gl/GLworld.h>
#include <engine/helpers/MathEx.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/Options.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Logging.h>
#include <GL/gl.h>
#include <GL/glew.h>
#include <SDL_error.h>
#include <SDL_video.h>
#include <stdbool.h>
#include <stdio.h>
#include <wchar.h>

SDL_GLContext ctx;

bool GL_PreInit()
{
	LogDebug("Pre-initializing OpenGL...\n");
	const bool msaaEnabled = GetState()->options.msaa != MSAA_NONE;
	if (SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, msaaEnabled) != 0)
	{
		LogError("Failed to set MSAA buffers attribute: %s\n", SDL_GetError());
	}
	if (msaaEnabled)
	{
		int mssaValue = 0;
		switch (GetState()->options.msaa)
		{
			case MSAA_2X:
				mssaValue = 2;
				break;
			case MSAA_4X:
				mssaValue = 4;
				break;
			case MSAA_8X:
				mssaValue = 8;
				break;
			default:
				LogError("OpenGL: Invalid MSAA value!");
				return false;
		}
		if (SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, mssaValue) != 0)
		{
			LogError("Failed to set MSAA samples attribute: %s\n", SDL_GetError());
		}
	}
	TestSDLFunction(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, GL_VERSION_MAJOR),
					"Failed to set OpenGL major version",
					GL_INIT_FAIL_MSG);
	TestSDLFunction(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, GL_VERSION_MINOR),
					"Failed to set OpenGL minor version",
					GL_INIT_FAIL_MSG);
	TestSDLFunction(SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1),
					"Failed to set OpenGL accelerated visual",
					GL_INIT_FAIL_MSG);
	TestSDLFunction(SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, GL_PROFILE),
					"Failed to set OpenGL profile",
					GL_INIT_FAIL_MSG);
	TestSDLFunction(SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1),
					"Failed to set OpenGL double buffer",
					GL_INIT_FAIL_MSG);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);

	return true;
}

bool GL_Init(SDL_Window *wnd)
{
	LogDebug("Initializing OpenGL renderer...\n");

	ctx = SDL_GL_CreateContext(wnd);
	if (ctx == NULL)
	{
		LogError("SDL_GL_CreateContext Error: %s\n", SDL_GetError());
		return false;
	}

	TestSDLFunction(SDL_GL_MakeCurrent(wnd, ctx), "Failed to make context current", GL_INIT_FAIL_MSG);

	TestSDLFunction_NonFatal(SDL_GL_SetSwapInterval(GetState()->options.vsync ? 1 : 0), "Failed to set VSync");

	glewExperimental = GL_TRUE; // Please expose OpenGL 3.x+ interfaces
	const GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		SDL_GL_DeleteContext(ctx);
		LogError("glewInit Failed with error %d\n", err);
		return false;
	}

	// Ensure we have GL 3.3 or higher
	if (!GL_VERSION_CHECK)
	{
		SDL_GL_DeleteContext(ctx);
		LogError("OpenGL: GL_VERSION_CHECK failed\n");
		return false;
	}

	if (GetState()->options.anisotropy != ANISOTROPY_NONE)
	{
		GLfloat requestedAnisotropy = 0;
		switch (GetState()->options.anisotropy)
		{
			case ANISOTROPY_2X:
				requestedAnisotropy = 2;
				break;
			case ANISOTROPY_4X:
				requestedAnisotropy = 4;
				break;
			case ANISOTROPY_8X:
				requestedAnisotropy = 8;
				break;
			case ANISOTROPY_16X:
				requestedAnisotropy = 16;
				break;
			default:
				LogError("OpenGL: Invalid anisotropy level!");
				return false;
		}
		GLfloat gpuMaxAnisotropy = 0;
		if (GLEW_EXT_texture_filter_anisotropic)
		{
			glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &gpuMaxAnisotropy);
		} else
		{
			LogWarning("GL: GPU does not support GL_EXT_texture_filter_anisotropic, but the user requested it.\n");
		}
		LogDebug("GL: GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT=%f\n", gpuMaxAnisotropy);
		anisotropyLevel = min(requestedAnisotropy, gpuMaxAnisotropy);
		if (requestedAnisotropy != anisotropyLevel)
		{
			LogWarning("GL: Actual anisotropy level of %f differs from requested value of %f. "
					   "GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT=%f\n",
					   anisotropyLevel,
					   requestedAnisotropy,
					   gpuMaxAnisotropy);
		}
	}


#ifdef BUILDSTYLE_DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(GL_DebugMessageCallback, NULL);

	int redSize, greenSize, blueSize, alphaSize;
	SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &redSize);
	SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, &greenSize);
	SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, &blueSize);
	SDL_GL_GetAttribute(SDL_GL_ALPHA_SIZE, &alphaSize);
	LogDebug("Window Framebuffer: R:%d G:%d B:%d A:%d\n", redSize, greenSize, blueSize, alphaSize);
#endif

	if (!GL_LoadShaders())
	{
		return false;
	}

	GL_InitObjects();

	if (!GL_InitFramebuffer())
	{
		return false;
	}

	glClearColor(0, 0, 0, 1);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glDisable(GL_SCISSOR_TEST);

	char *vendor = (char *)glGetString(GL_VENDOR);
	char *renderer = (char *)glGetString(GL_RENDERER);
	char *version = (char *)glGetString(GL_VERSION);
	char *shadingLanguage = (char *)glGetString(GL_SHADING_LANGUAGE_VERSION);

	LogInfo("OpenGL Initialized\n");
	LogInfo("OpenGL Vendor: %s\n", vendor);
	LogInfo("OpenGL Renderer: %s\n", renderer);
	LogInfo("OpenGL Version: %s\n", version);
	LogInfo("GLSL: %s\n", shadingLanguage);

	fflush(stdout);

	GL_Disable3D();

	return true;
}

void GL_DestroyGL()
{
	LogDebug("Cleaning up OpenGL renderer...\n");
	GL_DestroyFramebuffer();
	GL_DestroyShaders();
	GL_DestroyObjects();
	SDL_GL_DeleteContext(ctx);
}
