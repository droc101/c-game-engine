//
// Created by droc101 on 10/2/2024.
//

#ifdef BUILDSTYLE_DEBUG

#include <GL/glew.h>
#include <signal.h>
#include "../../Core/Logging.h"
#include "GLInternal.h"

#define BREAK_ON_ERROR

void GL_DebugMessageCallback(const GLenum source,
							 const GLenum type,
							 const GLuint id,
							 const GLenum severity,
							 GLsizei /*length*/,
							 const GLchar *msg,
							 const void * /*data*/)
{
	char *_source;
	char *_type;
	char *_severity;

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
	raise(SIGABRT);
#endif
}

#endif
