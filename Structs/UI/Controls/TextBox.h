//
// Created by droc101 on 1/7/25.
//

#ifndef TEXTBOX_H
#define TEXTBOX_H

#include "../../../defines.h"
#include "../../../Helpers/TextInputSystem.h"
#include "../UiStack.h"

typedef struct TextBoxData TextBoxData;

typedef void (*TextBoxCallback)(const char *text);

struct TextBoxData
{
	uint maxLength;
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
							  uint maxLength,
							  TextBoxCallback callback);

void DrawTextBox(const Control *c, ControlState state, Vector2 position);

void UpdateTextBox(UiStack *stack, Control *, Vector2 localMousePos, uint ctlIndex);

void DestroyTextBox(const Control *c);

void FocusTextBox(const Control *c);

void UnfocusTextBox(const Control *c);

void TextBoxTextInputCallback(TextInput *data, SDL_TextInputEvent *event);

#endif //TEXTBOX_H
