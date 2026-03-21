//
// Created by droc101 on 9/30/2024.
//

#include <engine/graphics/gl/GLdebug.h>
#include <engine/graphics/gl/GLframe.h>
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
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_video.h>
#include <stdbool.h>
#include <stdio.h>
#include <wchar.h>

SDL_GLContext ctx;

bool GL_PreInit()
{
	LogDebug("Pre-initializing OpenGL...\n");
	TestSDLFunction(SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0),
					"Failed to set OpenGL MSAA buffers",
					GL_INIT_FAIL_MSG);
	TestSDLFunction(SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0),
					"Failed to set OpenGL MSAA samples",
					GL_INIT_FAIL_MSG);
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
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
#ifdef BUILDSTYLE_DEBUG
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif


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

	GL_SetVsyncEnabled(GetState()->options.vsync);

	glewExperimental = GL_TRUE; // Please expose OpenGL 3.x+ interfaces
	const GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		SDL_GL_DestroyContext(ctx);
		LogError("glewInit Failed with error %d\n", err);
		return false;
	}

	if (!GL_VERSION_CHECK)
	{
		SDL_GL_DestroyContext(ctx);
		LogError("OpenGL: GL_VERSION_CHECK failed\n");
		return false;
	}

	if (!GLEW_ARB_framebuffer_object)
	{
		LogError("OpenGL device does not support GLEW_ARB_framebuffer_object!\n");
		return false;
	}

	GL_UpdateAnisotropyLevel();

#ifdef BUILDSTYLE_DEBUG
	int glContextFlags = 0;
	glGetIntegerv(GL_CONTEXT_FLAGS, &glContextFlags);
	if (glContextFlags & GL_CONTEXT_FLAG_DEBUG_BIT)
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(GL_DebugMessageCallback, NULL);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
	} else
	{
		LogWarning("OpenGL debugging was not enabled as GL_CONTEXT_FLAG_DEBUG_BIT didn't get set\n");
	}

	int redSize = 0;
	int greenSize = 0;
	int blueSize = 0;
	int alphaSize = 0;
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

	if (!GL_InitFramebuffer(GetState()->options.msaa))
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

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_MULTISAMPLE);

	return true;
}

void GL_DestroyGL()
{
	LogDebug("Cleaning up OpenGL renderer...\n");
	GL_DestroyFramebuffer();
	GL_DestroyShaders();
	GL_DestroyObjects();
	SDL_GL_DestroyContext(ctx);
}
