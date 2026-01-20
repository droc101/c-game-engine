//
// Created by droc101 on 11/20/25.
//

#include <cglm/cglm.h>
#include <engine/graphics/gl/GLdebug.h>
#include <engine/graphics/gl/GLframe.h>
#include <engine/graphics/gl/GLobjects.h>
#include <engine/graphics/gl/GLshaders.h>
#include <engine/graphics/RenderingHelpers.h>
#include <engine/subsystem/Logging.h>
#include <SDL_video.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define GL_COLOR_INTERNAL_FORMAT GL_RGB8
#define GL_COLOR_FORMAT GL_RGB
#define GL_COLOR_TYPE GL_UNSIGNED_BYTE
#define GL_DEPTH_FORMAT GL_DEPTH24_STENCIL8

GLuint frameBufferObject;
GLuint renderBufferObject;
GLuint framebufferColorTexture;
GL_Buffer *fullScreenQuadBuffer;

bool GL_InitFramebuffer()
{
	glGenFramebuffers(1, &frameBufferObject);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBufferObject);
	glGenRenderbuffers(1, &renderBufferObject);
	glBindRenderbuffer(GL_RENDERBUFFER, renderBufferObject);

	glGenTextures(1, &framebufferColorTexture);
	glBindTexture(GL_TEXTURE_2D, framebufferColorTexture);

	glTexImage2D(GL_TEXTURE_2D,
				 0,
				 GL_COLOR_INTERNAL_FORMAT,
				 DEF_WIDTH,
				 DEF_HEIGHT,
				 0,
				 GL_COLOR_FORMAT,
				 GL_COLOR_TYPE,
				 NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferColorTexture, 0);

	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_FORMAT, DEF_WIDTH, DEF_WIDTH);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderBufferObject);

	const GLenum frameBufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (frameBufferStatus != GL_FRAMEBUFFER_COMPLETE)
	{
		LogError("OpenGL framebuffer is incomplete, status is %u\n", frameBufferStatus);
		return false;
	}

#ifdef BUILDSTYLE_DEBUG
	int redSize, greenSize, blueSize, alphaSize, depthSize, stencilSize;
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
	LogDebug("Internal Framebuffer: R:%d G:%d B:%d A:%d D:%d S:%d\n",
			 redSize,
			 greenSize,
			 blueSize,
			 alphaSize,
			 depthSize,
			 stencilSize);
#endif

	glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
	glBindRenderbuffer(GL_RENDERBUFFER, GL_NONE);

	fullScreenQuadBuffer = GL_ConstructBuffer();
	GL_UseShader(framebufferShader);

	const float vertices[4][4] = {
		{-1, 1, 0, 1},
		{-1, -1, 0, 0},
		{1, -1, 1, 0},
		{1, 1, 1, 1},
	};

	const uint32_t indices[] = {0, 1, 2, 0, 2, 3};

	GL_BindBuffer(fullScreenQuadBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STREAM_DRAW);

	glVertexAttribPointer(framebufferPositionLoc, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void *)0);
	glEnableVertexAttribArray(framebufferPositionLoc);

	glVertexAttribPointer(framebufferUvLoc, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void *)(2 * sizeof(GLfloat)));
	glEnableVertexAttribArray(framebufferUvLoc);

	return true;
}

void GL_DestroyFramebuffer()
{
	glDeleteFramebuffers(1, &frameBufferObject);
	glDeleteRenderbuffers(1, &renderBufferObject);
	glDeleteTextures(1, &framebufferColorTexture);
	GL_DestroyBuffer(fullScreenQuadBuffer);
}

bool GL_FrameStart()
{
	GL_ResetDebugLines();
	glBindFramebuffer(GL_FRAMEBUFFER, frameBufferObject);
	glBindRenderbuffer(GL_RENDERBUFFER, renderBufferObject);
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
	glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
	glBindRenderbuffer(GL_RENDERBUFFER, GL_NONE);

	glDisable(GL_DEPTH_TEST);
	GL_UseShader(framebufferShader);
	GL_BindBuffer(fullScreenQuadBuffer);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, framebufferColorTexture);
	glUniform1i(framebufferTextureLoc, 0);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);

	SDL_GL_SwapWindow(GetGameWindow());
}

inline void GL_UpdateViewportSize()
{
	int vpWidth = 0;
	int vpHeight = 0;
	SDL_GL_GetDrawableSize(GetGameWindow(), &vpWidth, &vpHeight);
	glViewport(0, 0, vpWidth, vpHeight);

	glBindTexture(GL_TEXTURE_2D, framebufferColorTexture);
	glTexImage2D(GL_TEXTURE_2D,
				 0,
				 GL_COLOR_INTERNAL_FORMAT,
				 vpWidth,
				 vpHeight,
				 0,
				 GL_COLOR_FORMAT,
				 GL_COLOR_TYPE, NULL);

	GLint boundRbo = 0;
	glGetIntegerv(GL_RENDERBUFFER_BINDING, &boundRbo);

	glBindRenderbuffer(GL_RENDERBUFFER, renderBufferObject);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_FORMAT, vpWidth, vpHeight);
	glBindRenderbuffer(GL_RENDERBUFFER, boundRbo);
}
