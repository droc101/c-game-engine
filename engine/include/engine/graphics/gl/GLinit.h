//
// Created by droc101 on 9/30/2024.
//

#ifndef GAME_GLHELPER_H
#define GAME_GLHELPER_H

#include <cglm/types.h>
#include <SDL3/SDL_video.h>
#include <stdbool.h>

/// The major OpenGL version to use
#define GL_VERSION_MAJOR 3
/// The minor OpenGL version to use
#define GL_VERSION_MINOR 3
/// The OpenGL profile to use
#define GL_PROFILE SDL_GL_CONTEXT_PROFILE_CORE
/// The GLEW macro to use when checking for GL version support
#define GL_VERSION_CHECK GLEW_VERSION_3_3
/// A string describing the GL version
#define GL_VERSION_STRING "OpenGL 3.3 Core Profile"
/// The error message to show when OpenGL fails to start
#define GL_INIT_FAIL_MSG "Failed to start OpenGL. Your GPU or drivers may not support the " GL_VERSION_STRING "."

/**
 * Set SDL_GL flags (this must be done before the SDL window is created)
 */
bool GL_PreInit();

/**
 * Initialize OpenGL
 */
bool GL_Init(SDL_Window *wnd);

/**
 * Destroy the GL state
 */
void GL_DestroyGL();

#endif //GAME_GLHELPER_H
