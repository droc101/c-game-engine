//
// Created by droc101 on 1/7/25.
//

#include <engine/assets/AssetReader.h>
#include <engine/graphics/Drawing.h>
#include <engine/graphics/Font.h>
#include <engine/helpers/MathEx.h>
#include <engine/structs/Color.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Input.h>
#include <engine/subsystem/TextInputSystem.h>
#include <engine/subsystem/Timing.h>
#include <engine/uiStack/controls/TextBox.h>
#include <engine/uiStack/UiStack.h>
#include <SDL_events.h>
#include <SDL_scancode.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

Control *CreateTextBoxControl(const char *placeholder,
							  const Vector2 position,
							  const Vector2 size,
							  const ControlAnchor anchor,
							  const uint32_t maxLength,
							  const TextBoxCallback callback)
{
	Control *c = CreateEmptyControl();
	c->type = TEXTBOX;
	c->anchor = anchor;
	c->position = position;
	c->size = size;

	TextBoxData *data = calloc(1, sizeof(TextBoxData));
	CheckAlloc(data);
	c->controlData = data;
	data->maxLength = maxLength;
	data->callback = callback;
	data->text = calloc(1, maxLength + 1);
	CheckAlloc(data->text);
	data->input.userData = c;
	data->input.TextInput = TextBoxTextInputCallback;
	strcpy(data->placeholder, placeholder); // up to caller to ensure placeholder is not too long


	return c;
}

void DrawTextBox(const Control *control, ControlState /*state*/, Vector2 position)
{
	DrawNinePatchTexture(control->anchoredPosition, control->size, 8, 8, TEXTURE("interface/textbox"));

	const TextBoxData *data = (TextBoxData *)control->controlData;

	DrawTextAligned(strlen(data->text) == 0 ? data->placeholder : data->text,
					16,
					strlen(data->text) == 0 ? COLOR(0x7F000000) : COLOR_BLACK,
					v2(position.x + 6, position.y + 6),
					v2(control->size.x - 12, control->size.y - 12),
					FONT_HALIGN_LEFT,
					FONT_VALIGN_MIDDLE,
					smallFont);

	const Vector2 textSize = MeasureTextNChars(data->text, 16, smallFont, data->input.cursor);

	if (data->isActive && (GetTimeMs() % 1000) < 500)
	{
		DrawTextAligned("_",
						16,
						COLOR_BLACK,
						v2(position.x + 6 + textSize.x, position.y + 6 + 4),
						v2(12, control->size.y - 12),
						FONT_HALIGN_LEFT,
						FONT_VALIGN_MIDDLE,
						smallFont);
	}
}

void UpdateTextBox(UiStack *stack, Control *control, Vector2 /*localMousePosition*/, const uint32_t controlIndex)
{
	TextBoxData *data = (TextBoxData *)control->controlData;
	if (stack->focusedControl != controlIndex)
	{
		data->isActive = false;
		return;
	}
	data->isActive = true;

	if (IsKeyPressed(SDL_SCANCODE_LCTRL) || IsKeyPressed(SDL_SCANCODE_RCTRL))
	{
		int dir = 0;
		if (IsKeyJustPressed(SDL_SCANCODE_LEFT))
		{
			ConsumeKey(SDL_SCANCODE_LEFT);
			dir = -1;
		} else if (IsKeyJustPressed(SDL_SCANCODE_RIGHT))
		{
			ConsumeKey(SDL_SCANCODE_RIGHT);
			dir = 1;
		}
		if (dir != 0)
		{
			size_t i = data->input.cursor + dir;
			const size_t length = strlen(data->text);
			while (i > 0 && i < length && data->text[i] != ' ')
			{
				i += dir;
			}
			if (dir == 1)
			{
				i++; // jump to start of word instead of space
			}
			data->input.cursor = i;
		}
	} else
	{
		if (IsKeyJustPressed(SDL_SCANCODE_LEFT))
		{
			ConsumeKey(SDL_SCANCODE_LEFT);
			data->input.cursor -= 1;
		} else if (IsKeyJustPressed(SDL_SCANCODE_RIGHT))
		{
			ConsumeKey(SDL_SCANCODE_RIGHT);
			data->input.cursor += 1;
		}
	}

	data->input.cursor = min(data->input.cursor, strlen(data->text));

	if (IsKeyJustPressed(SDL_SCANCODE_HOME))
	{
		ConsumeKey(SDL_SCANCODE_HOME);
		data->input.cursor = 0;
	} else if (IsKeyJustPressed(SDL_SCANCODE_END))
	{
		ConsumeKey(SDL_SCANCODE_END);
		data->input.cursor = (int)strlen(data->text);
	}

	// handle backspace
	if (IsKeyJustPressed(SDL_SCANCODE_BACKSPACE))
	{
		if (data->input.cursor > 0)
		{
			ConsumeKey(SDL_SCANCODE_BACKSPACE);
			memmove(data->text + data->input.cursor - 1,
					data->text + data->input.cursor,
					strlen(data->text) - data->input.cursor + 1);
			data->input.cursor -= 1;
			if (data->callback != NULL)
			{
				data->callback(data->text);
			}
		}
	} else if (IsKeyJustPressed(SDL_SCANCODE_DELETE) && data->input.cursor < strlen(data->text))
	{
		ConsumeKey(SDL_SCANCODE_DELETE);
		memmove(data->text + data->input.cursor,
				data->text + data->input.cursor + 1,
				strlen(data->text) - data->input.cursor);
		if (data->callback != NULL)
		{
			data->callback(data->text);
		}
	}
}

void DestroyTextBox(const Control *control)
{
	const TextBoxData *textBoxData = control->controlData;
	free(textBoxData->text);
	free(control->controlData);
}

void FocusTextBox(const Control *control)
{
	SetTextInput(&((TextBoxData *)control->controlData)->input); // very readable yes
}

void UnfocusTextBox(const Control * /*control*/)
{
	StopTextInput();
}

void TextBoxTextInputCallback(TextInput *data, SDL_TextInputEvent *event)
{
	const TextBoxData *textBoxData = (TextBoxData *)((Control *)data->userData)->controlData;
	const size_t originalLen = strlen(textBoxData->text);

	size_t insertLen = strlen(event->text);

	// if the text is too long, truncate it
	if (originalLen + insertLen > textBoxData->maxLength)
	{
		insertLen = textBoxData->maxLength - originalLen;
	}

	memmove(textBoxData->text + data->cursor + insertLen,
			textBoxData->text + data->cursor,
			originalLen - data->cursor + 1);

	memccpy(textBoxData->text + data->cursor, event->text, 0, insertLen);

	data->cursor += insertLen;
	if (textBoxData->callback != NULL)
	{
		textBoxData->callback(textBoxData->text);
	}
}
