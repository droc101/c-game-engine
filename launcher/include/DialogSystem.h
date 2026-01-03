//
// Created by droc101 on 11/14/2025.
//

#ifndef GAME_DIALOGSYSTEM_H
#define GAME_DIALOGSYSTEM_H

#ifdef WIN32
#include <windows.h>
#else
#include <wchar.h>
#endif

/**
 * Initialize the dialog system
 */
void InitDialogSystem();

/**
 * Show an error dialog
 * @param message The message (body text)
 * @param title The title bar text and instruction (top) text
 */
void ErrorDialog(const wchar_t *message, const wchar_t *title);

#endif //GAME_DIALOGSYSTEM_H
