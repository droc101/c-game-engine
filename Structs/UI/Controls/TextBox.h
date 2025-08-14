//
// Created by droc101 on 1/7/25.
//

#ifndef TEXTBOX_H
#define TEXTBOX_H

#include <SDL_events.h>
#include <stdbool.h>
#include <stdint.h>
#include "../../../Helpers/TextInputSystem.h"
#include "../../Vector2.h"
#include "../UiStack.h"

typedef struct TextBoxData TextBoxData;

typedef void (*TextBoxCallback)(const char *text);

struct TextBoxData
{
	uint32_t maxLength;
	char *text;
	char placeholder[32];
	TextBoxCallback callback;
	TextInput input;
	bool isActive;
};

Control *CreateTextBoxControl(const char *placeholder,
							  Vector2 position,
							  Vector2 size,
							  ControlAnchor anchor,
							  uint32_t maxLength,
							  TextBoxCallback callback);

void DrawTextBox(const Control *control, ControlState state, Vector2 position);

void UpdateTextBox(UiStack *stack, Control *control, Vector2 localMousePosition, uint32_t controlIndex);

void DestroyTextBox(const Control *control);

void FocusTextBox(const Control *control);

void UnfocusTextBox(const Control *control);

void TextBoxTextInputCallback(TextInput *data, SDL_TextInputEvent *event);

#endif //TEXTBOX_H
