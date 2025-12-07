//
// Created by droc101 on 4/21/2024.
//

#ifndef GAME_ERROR_H
#define GAME_ERROR_H

/// Throw a fatal error
#define Error(error) _ErrorInternal(error, __FILE_NAME__, __LINE__, __PRETTY_FUNCTION__) // NOLINT(*-include-cleaner)

/**
 * Check if a pointer is NULL and if it is, call the error handler
 * @param ptr The pointer to check
 */
#define CheckAlloc(ptr) \
	if ((ptr) == NULL) _GameAllocFailure()

/**
 * Internal error handler for memory allocation failures
 */
_Noreturn void _GameAllocFailure(); // NOLINT(*-reserved-identifier)

/**
 * Internal error handler
 * @param error Error message
 * @param file File name
 * @param line Line number
 * @param function Function name
 * @warning Do not use this function directly, use the @c Error macro instead
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
 * Ask the user if they want to restart the program (or continue running)
 * @param title The title of the prompt
 * @param description The description/message of the prompt
 * @param yesBtn The text of the yes/restart button
 * @param noBtn The text of the no/continue button
 */
void PromptRelaunch(const char *title, const char *description, const char *yesBtn, const char *noBtn);

/**
 * Shows an error message saying that vk/gl failed to initialize and offers to switch to the other or exit
 */
_Noreturn void RenderInitError();

/**
 * Sets the signal handler to catch @c SIGSEGV and @c SIGFPE
 * @note This intentionally only functions in release mode
 */
void ErrorHandlerInit();

/**
 * Check if an SDL function failed (returned nonzero) and log the error
 * @param result The result of the SDL function
 * @param message The message to log (%s: sdl error)
 */
void TestSDLFunction_NonFatal(int result, const char *message);

/**
 * Test an SDL function for failure and log the error, terminating the program if it failed
 * @param result The result of the SDL function
 * @param message The log message
 * @param userMessage The user-facing crash message
 */
void TestSDLFunction(int result, const char *message, const char *userMessage);

#endif //GAME_ERROR_H
