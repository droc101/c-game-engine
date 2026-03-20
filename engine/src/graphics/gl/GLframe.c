//
// Created by droc101 on 11/20/25.
//

#include <cglm/cglm.h>
#include <engine/graphics/gl/GLdebug.h>
#include <engine/graphics/gl/GLframe.h>
#include <engine/graphics/gl/GLobjects.h>
#include <engine/graphics/RenderingHelpers.h>
#include <engine/helpers/MathEx.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/Options.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Logging.h>
#include <SDL3/SDL_video.h>
#include <stdbool.h>
#include <stddef.h>

#define GL_WORLD_FRAMEBUFFER_COLOR_INTERNAL_FORMAT GL_RGBA16F
#define GL_WORLD_FRAMEBUFFER_COLOR_FORMAT GL_RGBA
#define GL_WORLD_FRAMEBUFFER_COLOR_TYPE GL_FLOAT
#define GL_WORLD_FRAMEBUFFER_DEPTH_FORMAT GL_DEPTH24_STENCIL8

#define GL_UI_FRAMEBUFFER_COLOR_INTERNAL_FORMAT GL_RGBA8
#define GL_UI_FRAMEBUFFER_COLOR_FORMAT GL_RGBA
#define GL_UI_FRAMEBUFFER_COLOR_TYPE GL_UNSIGNED_BYTE

GLuint worldFrameBufferObject;
GLuint worldRenderBufferObject;
GLuint worldFramebufferColorTexture;

GLuint uiFrameBufferObject;
GLuint uiFramebufferColorTexture;

int GetActualMsaaSamples(const OptionsMsaa requested)
{
	const bool msaaEnabled = requested != MSAA_NONE;
	if (msaaEnabled)
	{
		int requestedMsaaValue = 0;
		switch (requested)
		{
			case MSAA_2X:
				requestedMsaaValue = 2;
				break;
			case MSAA_4X:
				requestedMsaaValue = 4;
				break;
			case MSAA_8X:
				requestedMsaaValue = 8;
				break;
			default:
				LogError("OpenGL: Invalid MSAA value!");
				return false;
		}

		GLint gpuMaxSamples = 0;
		glGetIntegerv(GL_MAX_SAMPLES, &gpuMaxSamples);
		LogDebug("GL: GL_MAX_SAMPLES=%d\n", gpuMaxSamples);
		const int msaaValue = min(requestedMsaaValue, gpuMaxSamples);
		if (msaaValue != requestedMsaaValue)
		{
			LogWarning("GL: Actual MSAA samples of %d differs from requested value of %d. "
					   "GL_MAX_SAMPLES=%d\n",
					   msaaValue,
					   requestedMsaaValue,
					   gpuMaxSamples);
		}

		return msaaValue;
	}
	return 1;
}

void GL_SetVsyncEnabled(const bool enable)
{
	SDL_GL_SetSwapInterval(enable ? 1 : 0);
}

bool GL_InitFramebuffer(const OptionsMsaa msaaSamples)
{
	glMsaaSamples = GetActualMsaaSamples(msaaSamples);

	glGenFramebuffers(1, &worldFrameBufferObject);
	glBindFramebuffer(GL_FRAMEBUFFER, worldFrameBufferObject);
	glGenRenderbuffers(1, &worldRenderBufferObject);
	glBindRenderbuffer(GL_RENDERBUFFER, worldRenderBufferObject);

	glGenTextures(1, &worldFramebufferColorTexture);
	if (glMsaaSamples != 0)
	{
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, worldFramebufferColorTexture);

		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE,
								glMsaaSamples,
								GL_WORLD_FRAMEBUFFER_COLOR_INTERNAL_FORMAT,
								DEF_WIDTH,
								DEF_HEIGHT,
								GL_TRUE);

		// Multisample textures do not have filter or repeat,
		// and it doesn't matter here anyway since we just blit it to the main buffer

		glFramebufferTexture2D(GL_FRAMEBUFFER,
							   GL_COLOR_ATTACHMENT0,
							   GL_TEXTURE_2D_MULTISAMPLE,
							   worldFramebufferColorTexture,
							   0);

		glRenderbufferStorageMultisample(GL_RENDERBUFFER,
										 glMsaaSamples,
										 GL_WORLD_FRAMEBUFFER_DEPTH_FORMAT,
										 DEF_WIDTH,
										 DEF_WIDTH);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER,
								  GL_DEPTH_STENCIL_ATTACHMENT,
								  GL_RENDERBUFFER,
								  worldRenderBufferObject);
	} else
	{
		glBindTexture(GL_TEXTURE_2D, worldFramebufferColorTexture);

		glTexImage2D(GL_TEXTURE_2D,
					 0,
					 GL_WORLD_FRAMEBUFFER_COLOR_INTERNAL_FORMAT,
					 DEF_WIDTH,
					 DEF_HEIGHT,
					 0,
					 GL_WORLD_FRAMEBUFFER_COLOR_FORMAT,
					 GL_WORLD_FRAMEBUFFER_COLOR_TYPE,
					 NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, worldFramebufferColorTexture, 0);

		glRenderbufferStorage(GL_RENDERBUFFER, GL_WORLD_FRAMEBUFFER_DEPTH_FORMAT, DEF_WIDTH, DEF_WIDTH);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER,
								  GL_DEPTH_STENCIL_ATTACHMENT,
								  GL_RENDERBUFFER,
								  worldRenderBufferObject);
	}

	const GLenum worldFrameBufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (worldFrameBufferStatus != GL_FRAMEBUFFER_COMPLETE)
	{
		LogError("OpenGL world framebuffer is incomplete, status is %u\n", worldFrameBufferStatus);
		return false;
	}

#ifdef BUILDSTYLE_DEBUG
	int redSize = 0;
	int greenSize = 0;
	int blueSize = 0;
	int alphaSize = 0;
	int depthSize = 0;
	int stencilSize = 0;
	glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,
										  GL_COLOR_ATTACHMENT0,
										  GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE,
										  &redSize);
	glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,
										  GL_COLOR_ATTACHMENT0,
										  GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE,
										  &greenSize);
	glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,
										  GL_COLOR_ATTACHMENT0,
										  GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE,
										  &blueSize);
	glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,
										  GL_COLOR_ATTACHMENT0,
										  GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE,
										  &alphaSize);
	glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,
										  GL_DEPTH_ATTACHMENT,
										  GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE,
										  &depthSize);
	glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,
										  GL_DEPTH_ATTACHMENT,
										  GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE,
										  &stencilSize);
	LogDebug("Internal World Framebuffer: R:%d G:%d B:%d A:%d D:%d S:%d\n",
			 redSize,
			 greenSize,
			 blueSize,
			 alphaSize,
			 depthSize,
			 stencilSize);
#endif

	glGenFramebuffers(1, &uiFrameBufferObject);
	glBindFramebuffer(GL_FRAMEBUFFER, uiFrameBufferObject);
	glBindRenderbuffer(GL_RENDERBUFFER, GL_NONE);

	glGenTextures(1, &uiFramebufferColorTexture);
	glBindTexture(GL_TEXTURE_2D, uiFramebufferColorTexture);

	glTexImage2D(GL_TEXTURE_2D,
				 0,
				 GL_UI_FRAMEBUFFER_COLOR_INTERNAL_FORMAT,
				 DEF_WIDTH,
				 DEF_HEIGHT,
				 0,
				 GL_UI_FRAMEBUFFER_COLOR_FORMAT,
				 GL_UI_FRAMEBUFFER_COLOR_TYPE,
				 NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, uiFramebufferColorTexture, 0);

	const GLenum uiFrameBufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (uiFrameBufferStatus != GL_FRAMEBUFFER_COMPLETE)
	{
		LogError("OpenGL world framebuffer is incomplete, status is %u\n", uiFrameBufferStatus);
		return false;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
	glBindRenderbuffer(GL_RENDERBUFFER, GL_NONE);

	return true;
}

void GL_DestroyFramebuffer()
{
	glDeleteFramebuffers(1, &worldFrameBufferObject);
	glDeleteRenderbuffers(1, &worldRenderBufferObject);
	glDeleteTextures(1, &worldFramebufferColorTexture);
}

bool GL_FrameStart()
{
	GL_ResetDebugLines();

	if ((rendererQueuedActions & QUEUED_ACTION_RECREATE_FRAMEBUFFERS) != 0)
	{
		GL_DestroyFramebuffer();
		GL_InitFramebuffer(GetState()->options.msaa);
		GL_UpdateViewportSize();
		rendererQueuedActions &= ~QUEUED_ACTION_RECREATE_FRAMEBUFFERS;
	}

	if ((rendererQueuedActions & QUEUED_ACTION_CLEAR_ALL_TEXTURES) != 0)
	{
		GL_DeleteAllTextures();
		GL_UpdateAnisotropyLevel();
		rendererQueuedActions &= ~QUEUED_ACTION_CLEAR_ALL_TEXTURES;
	}

	if ((rendererQueuedActions & QUEUED_ACTION_CLEAR_ALL_MODELS) != 0)
	{
		GL_DestroyAllModels();
		rendererQueuedActions &= ~QUEUED_ACTION_CLEAR_ALL_MODELS;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, uiFrameBufferObject);
	glBindRenderbuffer(GL_RENDERBUFFER, GL_NONE);
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

inline void GL_FrameEnd()
{
	const Vector2 wndSize = ActualWindowSizeIgnoreDPI();
	glBindFramebuffer(GL_READ_FRAMEBUFFER, uiFrameBufferObject);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, GL_NONE);
	glBlitFramebuffer(0,
					  0,
					  (GLint)wndSize.x,
					  (GLint)wndSize.y,
					  0,
					  0,
					  (GLint)wndSize.x,
					  (GLint)wndSize.y,
					  GL_COLOR_BUFFER_BIT,
					  GL_NEAREST);

	SDL_GL_SwapWindow(GetGameWindow());
}

inline void GL_UpdateViewportSize()
{
	const Vector2 windowSize = ActualWindowSizeIgnoreDPI();
	glViewport(0, 0, (GLsizei)windowSize.x, (GLsizei)windowSize.y);

	GLint boundRbo = GL_NONE;
	GLint boundFbo = GL_NONE;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &boundFbo);
	glGetIntegerv(GL_RENDERBUFFER_BINDING, &boundRbo);
	glBindFramebuffer(GL_FRAMEBUFFER, worldFrameBufferObject);
	glBindRenderbuffer(GL_RENDERBUFFER, worldRenderBufferObject);

	if (glMsaaSamples != 0)
	{
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, worldFramebufferColorTexture);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE,
								glMsaaSamples,
								GL_WORLD_FRAMEBUFFER_COLOR_INTERNAL_FORMAT,
								(GLsizei)windowSize.x,
								(GLsizei)windowSize.y,
								GL_TRUE);

		glRenderbufferStorageMultisample(GL_RENDERBUFFER,
										 glMsaaSamples,
										 GL_WORLD_FRAMEBUFFER_DEPTH_FORMAT,
										 (GLsizei)windowSize.x,
										 (GLsizei)windowSize.y);
	} else
	{
		glBindTexture(GL_TEXTURE_2D, worldFramebufferColorTexture);
		glTexImage2D(GL_TEXTURE_2D,
					 0,
					 GL_WORLD_FRAMEBUFFER_COLOR_INTERNAL_FORMAT,
					 (GLsizei)windowSize.x,
					 (GLsizei)windowSize.y,
					 0,
					 GL_WORLD_FRAMEBUFFER_COLOR_FORMAT,
					 GL_WORLD_FRAMEBUFFER_COLOR_TYPE,
					 NULL);

		glRenderbufferStorage(GL_RENDERBUFFER,
							  GL_WORLD_FRAMEBUFFER_DEPTH_FORMAT,
							  (GLsizei)windowSize.x,
							  (GLsizei)windowSize.y);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, uiFrameBufferObject);
	glBindRenderbuffer(GL_RENDERBUFFER, GL_NONE);

	glBindTexture(GL_TEXTURE_2D, uiFramebufferColorTexture);
	glTexImage2D(GL_TEXTURE_2D,
				 0,
				 GL_UI_FRAMEBUFFER_COLOR_INTERNAL_FORMAT,
				 (GLsizei)windowSize.x,
				 (GLsizei)windowSize.y,
				 0,
				 GL_UI_FRAMEBUFFER_COLOR_FORMAT,
				 GL_UI_FRAMEBUFFER_COLOR_TYPE,
				 NULL);

	glBindFramebuffer(GL_FRAMEBUFFER, boundFbo);
	glBindRenderbuffer(GL_RENDERBUFFER, boundRbo);
}

void GL_Begin3DPass()
{
	glBindFramebuffer(GL_FRAMEBUFFER, worldFrameBufferObject);
	glBindRenderbuffer(GL_RENDERBUFFER, worldRenderBufferObject);
}

void GL_End3DPass()
{
	const Vector2 wndSize = ActualWindowSizeIgnoreDPI();
	glBindFramebuffer(GL_READ_FRAMEBUFFER, worldFrameBufferObject);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, uiFrameBufferObject);
	glBlitFramebuffer(0,
					  0,
					  (GLint)wndSize.x,
					  (GLint)wndSize.y,
					  0,
					  0,
					  (GLint)wndSize.x,
					  (GLint)wndSize.y,
					  GL_COLOR_BUFFER_BIT,
					  GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, uiFrameBufferObject);
	glBindRenderbuffer(GL_RENDERBUFFER, GL_NONE);
}
