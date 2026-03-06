//
// Created by droc101 on 10/2/2024.
//

#include <engine/graphics/gl/GLdebug.h>
#include <engine/graphics/gl/GLobjects.h>
#include <engine/graphics/gl/GLshaders.h>
#include <engine/structs/Color.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Logging.h>
#include <GL/glew.h>
#include <joltc/Math/Vector3.h>
#include <signal.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define BREAK_ON_ERROR

#define GL_DEBUG_LINES_BUFFER_RESIZE_INCREMENT 8192

GL_DebugLine *glDebugLines = NULL;
size_t debugLinesCapacity = 0;
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

void GL_AddDebugLine(const Vector3 start, const Vector3 end, const Color color)
{
	if (numDebugLines >= debugLinesCapacity)
	{
		LogDebug("Resizing GL debug lines buffer from %zu to %zu\n",
				 debugLinesCapacity,
				 debugLinesCapacity + GL_DEBUG_LINES_BUFFER_RESIZE_INCREMENT);
		void *newAlloc = realloc(glDebugLines,
								 sizeof(GL_DebugLine) * (debugLinesCapacity + GL_DEBUG_LINES_BUFFER_RESIZE_INCREMENT));
		CheckAlloc(newAlloc);
		glDebugLines = newAlloc;
		debugLinesCapacity += GL_DEBUG_LINES_BUFFER_RESIZE_INCREMENT;
	}
	const Vector3 colorVec = {color.r, color.g, color.b};

	GL_DebugLine *line = &glDebugLines[numDebugLines];
	line->start = start;
	line->startColor = colorVec;
	line->end = end;
	line->endColor = colorVec;
	numDebugLines++;
}

void GL_DrawDebugLines()
{
	glDisable(GL_LINE_SMOOTH);

	GL_UseShader(debugShader);

	glBindBufferBase(GL_UNIFORM_BUFFER, debugSharedUniformsLoc, sharedUniformBuffer);

	const GLsizeiptr linesBufferSize = (GLsizeiptr)(sizeof(GL_DebugLine) * numDebugLines);

	glBindBuffer(GL_ARRAY_BUFFER, glBuffer->vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, linesBufferSize, glDebugLines, GL_STREAM_DRAW);

	glVertexAttribPointer(debugVertexLoc, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void *)0);
	glEnableVertexAttribArray(debugVertexLoc);
	glVertexAttribPointer(debugColorLoc, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(debugColorLoc);

	glLineWidth(2.0f);
	glDrawArrays(GL_LINES, 0, (GLsizei)numDebugLines * 2);
}

void GL_ResetDebugLines()
{
	numDebugLines = 0;
}
