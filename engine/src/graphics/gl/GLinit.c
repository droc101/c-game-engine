//
// Created by droc101 on 9/30/2024.
//

#include <engine/graphics/gl/GLdebug.h>
#include <engine/graphics/gl/GLinit.h>
#include <engine/graphics/gl/GLobjects.h>
#include <engine/graphics/gl/GLshaders.h>
#include <engine/graphics/gl/GLworld.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/Options.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Logging.h>
#include <GL/gl.h>
#include <GL/glew.h>
#include <SDL_error.h>
#include <SDL_video.h>
#include <stdbool.h>
#include <stdint.h>
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
				GL_Error("Invalid MSAA value!");
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
	TestSDLFunction(SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8), "Failed to set OpenGL red-size", "Failed to start OpenGL");
	TestSDLFunction(SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8), "Failed to set OpenGL green-size", GL_INIT_FAIL_MSG);
	TestSDLFunction(SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8), "Failed to set OpenGL blue-size", GL_INIT_FAIL_MSG);
	TestSDLFunction(SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8), "Failed to set OpenGL alpha-size", GL_INIT_FAIL_MSG);
	TestSDLFunction(SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24),
					"Failed to set OpenGL depth buffer size",
					GL_INIT_FAIL_MSG);

	return true;
}

bool GL_Init(SDL_Window *wnd)
{
	LogDebug("Initializing OpenGL renderer...\n");

	ctx = SDL_GL_CreateContext(wnd);
	if (ctx == NULL)
	{
		LogError("SDL_GL_CreateContext Error: %s\n", SDL_GetError());
		GL_Error("Failed to create OpenGL context");
		return false;
	}

	TestSDLFunction_NonFatal(SDL_GL_SetSwapInterval(GetState()->options.vsync ? 1 : 0), "Failed to set VSync");

	glewExperimental = GL_TRUE; // Please expose OpenGL 3.x+ interfaces
	const GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		SDL_GL_DeleteContext(ctx);
		GL_Error(GL_INIT_FAIL_MSG);
		return false;
	}

	// Ensure we have GL 3.3 or higher
	if (!GL_VERSION_CHECK)
	{
		SDL_GL_DeleteContext(ctx);
		GL_Error(GL_INIT_FAIL_MSG);
		return false;
	}


#ifdef BUILDSTYLE_DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(GL_DebugMessageCallback, NULL);
#endif

	if (!GL_LoadShaders())
	{
		return false;
	}

	GL_InitObjects();

	glClearColor(0, 0, 0, 1);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);
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
	GL_DestroyShaders();
	GL_DestroyObjects();
	SDL_GL_DeleteContext(ctx);
}
