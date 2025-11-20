//
// Created by droc101 on 10/2/2024.
//

#include <engine/graphics/gl/GLdebug.h>
#include <engine/graphics/gl/GLobjects.h>
#include <engine/graphics/gl/GLshaders.h>
#include <engine/structs/Color.h>
#include <engine/subsystem/Logging.h>
#include <GL/glew.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define BREAK_ON_ERROR

GL_DebugLine glDebugLines[GL_MAX_DEBUG_LINES_PER_FRAME];
size_t numDebugLines = 0;

void GL_DebugMessageCallback(const GLenum source,
							 const GLenum type,
							 const GLuint id,
							 const GLenum severity,
							 GLsizei /*length*/,
							 const GLchar *msg,
							 const void * /*data*/)
{
	char *_source = NULL;
	char *_type = NULL;
	char *_severity = NULL;

	if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
	{
		return; // shut up
	}

	switch (source)
	{
		case GL_DEBUG_SOURCE_API:
			_source = "API";
			break;

		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
			_source = "WINDOW SYSTEM";
			break;

		case GL_DEBUG_SOURCE_SHADER_COMPILER:
			_source = "SHADER COMPILER";
			break;

		case GL_DEBUG_SOURCE_THIRD_PARTY:
			_source = "THIRD PARTY";
			break;

		case GL_DEBUG_SOURCE_APPLICATION:
			_source = "APPLICATION";
			break;

		case GL_DEBUG_SOURCE_OTHER:
		default:
			_source = "UNKNOWN";
			break;
	}

	switch (type)
	{
		case GL_DEBUG_TYPE_ERROR:
			_type = "ERROR";
			break;

		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
			_type = "DEPRECATED BEHAVIOR";
			break;

		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
			_type = "UNDEFINED BEHAVIOR";
			break;

		case GL_DEBUG_TYPE_PORTABILITY:
			_type = "PORTABILITY";
			break;

		case GL_DEBUG_TYPE_PERFORMANCE:
			_type = "PERFORMANCE";
			break;

		case GL_DEBUG_TYPE_OTHER:
			_type = "OTHER";
			break;

		case GL_DEBUG_TYPE_MARKER:
			_type = "MARKER";
			break;

		default:
			_type = "UNKNOWN";
			break;
	}

	switch (severity)
	{
		case GL_DEBUG_SEVERITY_HIGH:
			_severity = "HIGH";
			break;

		case GL_DEBUG_SEVERITY_MEDIUM:
			_severity = "MEDIUM";
			break;

		case GL_DEBUG_SEVERITY_LOW:
			_severity = "LOW";
			break;

		// ReSharper disable once CppDFAUnreachableCode
		case GL_DEBUG_SEVERITY_NOTIFICATION:
			_severity = "NOTIFICATION";
			break;

		default:
			_severity = "UNKNOWN";
			break;
	}

	LogDebug("%d: %s of %s severity, raised from %s: %s\n", id, _type, _severity, _source, msg);

#ifdef BREAK_ON_ERROR
	// If you hit this "breakpoint", an OpenGL error has been printed to the console,
	// and the corresponding GL call should be on the call stack.
	raise(SIGABRT);
#endif
}

void GL_DrawDebugLine(GL_DebugLine *line)
{
	glDisable(GL_LINE_SMOOTH);

	glUseProgram(debugShader->program);

	glBindBufferBase(GL_UNIFORM_BUFFER,
					 glGetUniformBlockIndex(debugShader->program, "SharedUniforms"),
					 sharedUniformBuffer);

	glUniform4fv(debugColorLoc, 1, COLOR_TO_ARR(line->color));

	// Calculate the 2 corner vertices of each point for a thick line
	const float vertices[2][3] = {
		{line->start.x, line->start.y, line->start.z},
		{line->end.x, line->end.y, line->end.z},
	};

	const uint32_t indices[] = {0, 1};

	glBindVertexArray(glBuffer->vertexArrayObject);

	glBindBuffer(GL_ARRAY_BUFFER, glBuffer->vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glBuffer->elementBufferObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STREAM_DRAW);

	const GLint posAttrLoc = glGetAttribLocation(uiColoredShader->program, "VERTEX");
	glVertexAttribPointer(posAttrLoc, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void *)0);
	glEnableVertexAttribArray(posAttrLoc);

	glLineWidth(2.0f);
	glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, NULL);
}

void GL_AddDebugLine(const Vector3 start, const Vector3 end, const Color color)
{
	if (numDebugLines >= GL_MAX_DEBUG_LINES_PER_FRAME)
	{
		LogError("Tried to add a GL debug line, but there were no free slots!\n");
		return;
	}
	GL_DebugLine *line = &glDebugLines[numDebugLines];
	line->start = start;
	line->end = end;
	line->color = color;
	numDebugLines++;
}

void GL_DrawDebugLines()
{
	for (size_t i = 0; i < numDebugLines; i++)
	{
		GL_DrawDebugLine(&glDebugLines[i]);
	}
}

void GL_ResetDebugLines()
{
	numDebugLines = 0;
}

void GL_Error(const char *error)
{
	LogError("OpenGL Error: %s\n", error);
	strcpy(glLastError, error);
}
