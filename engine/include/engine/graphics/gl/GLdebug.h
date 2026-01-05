//
// Created by droc101 on 11/20/25.
//

#ifndef GAME_GLDEBUG_H
#define GAME_GLDEBUG_H

#include <cglm/types.h>
#include <engine/graphics/gl/GLobjects.h>
#include <engine/structs/Color.h>
#include <GL/glew.h>
#include <joltc/Math/Vector3.h>
#include <stddef.h>

#define GL_MAX_DEBUG_LINES_PER_FRAME 8192

/**
 * Log an OpenGL error
 * @param error the error message
 */
void GL_Error(const char *error);


/**
 * Debug message callback for OpenGL
 * @param source The source of the message
 * @param type The type of the message
 * @param id The ID of the message
 * @param severity The severity of the message
 * @param length The length of the message
 * @param msg The message
 * @param data Extra data
 */
void GL_DebugMessageCallback(GLenum source,
							 GLenum type,
							 GLuint id,
							 GLenum severity,
							 GLsizei length,
							 const GLchar *msg,
							 const void *data);

/**
 * Add a debug line for the current frame
 * @param start The first vertex of the line
 * @param end The second vertex of the line
 * @param color The color of the line
 */
void GL_AddDebugLine(Vector3 start, Vector3 end, Color color);

/**
 * Draw a debug line
 * @param line The line to draw
 */
void GL_DrawDebugLine(GL_DebugLine *line);

/**
 * Clear all debug lines
 */
void GL_ResetDebugLines();

/**
 * Draw all debug lines
 */
void GL_DrawDebugLines();

#endif //GAME_GLDEBUG_H
