//
// Created by droc101 on 4/21/2024.
//

/**
 * @file Error.h
 * Error reporting and handling
 */

#ifndef GAME_ERROR_H
#define GAME_ERROR_H

/**
 * Throw a fatal error with a detailed error handler
 * @param error The error message
 */
#define Error(error) _ErrorInternal(error, __FILE__, __LINE__, __PRETTY_FUNCTION__) // NOLINT(*-include-cleaner)

/**
 * Check if a pointer is NULL and if it is, call the error handler
 * @param ptr The pointer to check
 */
#define CheckAlloc(ptr) \
	if ((ptr) == NULL) _GameAllocFailure()

/**
 * Internal error handler for memory allocation failures
 * @private
 */
_Noreturn void _GameAllocFailure(); // NOLINT(*-reserved-identifier)

/**
 * Internal error handler
 * @param error Error message
 * @param file File name
 * @param line Line number
 * @param function Function name
 * @warning Do not use this function directly, use the @c Error macro instead
 * @private
 */
// NOLINTNEXTLINE(*-reserved-identifier)
_Noreturn void _ErrorInternal(char *error, const char *file, int line, const char *function);

/**
 * Friendly error handler
 * @param title Friendly title
 * @param description Friendly description
 */
_Noreturn void FriendlyError(const char *title, const char *description);

/**
 * Non-terminating warning message box
 * @param title Message box title
 * @param description Warning text
 */
void ShowWarning(const char *title, const char *description);

/**
 * Sets the signal handler to catch @c SIGSEGV and @c SIGFPE
 * @note This intentionally only functions in release mode
 * @private
 */
void ErrorHandlerInit();

#endif //GAME_ERROR_H
