//
// Created by droc101 on 11/5/24.
//

#ifndef GAME_LOGGING_H
#define GAME_LOGGING_H

#include <stdbool.h>

#define FLUSH_ON_INFO true
#define FLUSH_ON_DEBUG true
#define FLUSH_ON_WARNING true
#define FLUSH_ON_ERROR true

#ifndef LOGGER_INFO_COLOR
#define LOGGER_INFO_COLOR 39
#endif
#ifndef LOGGER_DEBUG_COLOR
#define LOGGER_DEBUG_COLOR 39
#endif
#ifndef LOGGER_WARNING_COLOR
#define LOGGER_WARNING_COLOR 33
#endif
#ifndef LOGGER_ERROR_COLOR
#define LOGGER_ERROR_COLOR 31
#endif

/**
 * Initialize the logging system
 */
void LogInit();

/**
 * Destroy the logging system
 */
void LogDestroy();

/**
 * Internal handler for Log* macros
 * @param type The type of log, such as INFO or ERROR
 * @param color The ANSI foreground color code
 * @param flush Whether to flush output after printing
 * @param message The message format string
 * @param ... Format arguments
 */
void LogInternal(const char *type, int color, bool flush, const char *message, ...);

/**
 * Log an info message
 * @param ... Format arguments
 */
#define LogInfo(...) LogInternal("INFO", LOGGER_INFO_COLOR, FLUSH_ON_INFO, __VA_ARGS__)

#ifdef BUILDSTYLE_DEBUG
/**
 * Log an info message, but only in debug builds
 * @param ... Format arguments
 */
#define LogDebug(...) LogInternal("DEBUG", LOGGER_DEBUG_COLOR, FLUSH_ON_DEBUG, __VA_ARGS__)
#else
#define LogDebug(...)
#endif

/**
 * Log a warning message
 * @param ... Format arguments
 */
#define LogWarning(...) LogInternal("WARN", LOGGER_WARNING_COLOR, FLUSH_ON_WARNING, __VA_ARGS__)

/**
 * Log an error message
 * @param ... Format arguments
 */
#define LogError(...) LogInternal("ERROR", LOGGER_ERROR_COLOR, FLUSH_ON_ERROR, __VA_ARGS__)

#endif //GAME_LOGGING_H
