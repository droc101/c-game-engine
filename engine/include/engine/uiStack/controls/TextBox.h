//
// Created by droc101 on 1/7/25.
//

/**
 * @file TextBox.h
 * Text box control
 */

#ifndef TEXTBOX_H
#define TEXTBOX_H

#include <engine/structs/Vector2.h>
#include <engine/subsystem/TextInputSystem.h>
#include <engine/uiStack/UiStack.h>
#include <SDL3/SDL_events.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct TextBoxData TextBoxData;

/**
 * Callback signature for when a TextBox's text changes
 * @param text The new text
 */
typedef void (*TextBoxCallback)(const char *text);

/// @private
struct TextBoxData
{
	uint32_t maxLength;
	char *text;
	char *ime_edit;
	char placeholder[32];
	TextBoxCallback callback;
	TextInput input;
	bool isActive;
};

/**
 * Create a text box control
 * @param placeholder The placeholder text
 * @param position The position
 * @param size The size
 * @param anchor The positioning anchor
 * @param maxLength The max text length
 * @param callback The text changed callback
 * @param tooltip The tooltip
 */
Control *CreateTextBoxControl(const char *placeholder,
							  Vector2 position,
							  Vector2 size,
							  ControlAnchor anchor,
							  uint32_t maxLength,
							  TextBoxCallback callback,
							  char *tooltip);

/// @private
void DrawTextBox(const Control *control, ControlState state, Vector2 position);

/// @private
void UpdateTextBox(UiStack *stack, Control *control, Vector2 localMousePosition, uint32_t controlIndex);

/// @private
void DestroyTextBox(const Control *control);

/// @private
void FocusTextBox(const Control *control);

/// @private
void UnfocusTextBox(const Control *control);

/// @private
void TextBoxTextEditingCallback(TextInput *data, SDL_TextEditingEvent *event);

/// @private
void TextBoxTextInputCallback(TextInput *data, SDL_TextInputEvent *event);

#endif //TEXTBOX_H
