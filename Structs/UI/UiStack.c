//
// Created by droc101 on 10/7/2024.
//

#include "UiStack.h"
#include "../../Helpers/Core/AssetReader.h"
#include "../../Helpers/Core/Error.h"
#include "../../Helpers/Core/Input.h"
#include "../../Helpers/Core/MathEx.h"
#include "../../Helpers/Graphics/Drawing.h"
#include "../../Helpers/Graphics/RenderingHelpers.h"
#include "../GlobalState.h"
#include "../Vector2.h"

#include "Controls/Button.h"
#include "Controls/CheckBox.h"
#include "Controls/RadioButton.h"
#include "Controls/Slider.h"
#include "Controls/TextBox.h"

typedef void (*ControlDrawFunc)(const Control *, ControlState state, Vector2 position);
typedef void (*ControlUpdateFunc)(UiStack *stack, Control *, Vector2 localMousePos, uint ctlIndex);
typedef void (*ControlDestroyFunc)(const Control *);
typedef void (*ControlFocusFunc)(const Control *);
typedef void (*ControlUnfocusFunc)(const Control *);

/**
 * Destroy functions for each control type
 */
const ControlDestroyFunc ControlDestroyFuncs[CONTROL_TYPE_COUNT] = {
	DestroyButton, // BUTTON
	DestroySlider, // SLIDER
	DestroyCheckbox, // CHECKBOX
	DestroyRadioButton, // RADIO_BUTTON
	DestroyTextBox, // TEXTBOX
};

/**
 * Draw functions for each control type
 */
const ControlDrawFunc ControlDrawFuncs[CONTROL_TYPE_COUNT] = {
	DrawButton, // BUTTON
	DrawSlider, // SLIDER
	DrawCheckbox, // CHECKBOX
	DrawRadioButton, // RADIO_BUTTON
	DrawTextBox, // TEXTBOX
};

/**
 * Update functions for each control type
 */
const ControlUpdateFunc ControlUpdateFuncs[CONTROL_TYPE_COUNT] = {
	UpdateButton, // BUTTON
	UpdateSlider, // SLIDER
	UpdateCheckbox, // CHECKBOX
	UpdateRadioButton, // RADIO_BUTTON
	UpdateTextBox, // TEXTBOX
};

/**
 * Focus functions for each control type
 */
const ControlFocusFunc ControlFocusFuncs[CONTROL_TYPE_COUNT] = {
	NULL, // BUTTON
	NULL, // SLIDER
	NULL, // CHECKBOX
	NULL, // RADIO_BUTTON
	FocusTextBox, // TEXTBOX
};

/**
 * Unfocus functions for each control type
 */
const ControlUnfocusFunc ControlUnfocusFuncs[CONTROL_TYPE_COUNT] = {
	NULL, // BUTTON
	NULL, // SLIDER
	NULL, // CHECKBOX
	NULL, // RADIO_BUTTON
	UnfocusTextBox, // TEXTBOX
};

UiStack *CreateUiStack()
{
	UiStack *stack = malloc(sizeof(UiStack));
	CheckAlloc(stack);
	ListInit(stack->Controls, LIST_POINTER);
	stack->ActiveControl = -1;
	stack->ActiveControlState = NORMAL;
	stack->focusedControl = -1;
	UiStackResetFocus(stack);
	return stack;
}

void DestroyUiStack(UiStack *stack)
{
	for (int i = 0; i < stack->Controls.length; i++)
	{
		const Control *c = ListGetPointer(stack->Controls, i);
		ControlDestroyFuncs[c->type](c);
	}
	ListAndContentsFree(stack->Controls);
	free(stack);
	stack = NULL;
}

bool ProcessUiStack(UiStack *stack)
{
	const Vector2 mousePos = GetMousePos();

	if (stack->focusedControl != -1)
	{
		Control *c = ListGetPointer(stack->Controls, stack->focusedControl);
		ControlUpdateFuncs[c->type](stack,
									c,
									v2(mousePos.x - c->position.x, mousePos.y - c->position.y),
									stack->focusedControl);
	}
	if (stack->ActiveControl != -1)
	{
		Control *c = ListGetPointer(stack->Controls, stack->ActiveControl);
		ControlUpdateFuncs[c->type](stack,
									c,
									v2(mousePos.x - c->position.x, mousePos.y - c->position.y),
									stack->ActiveControl);
	}


	for (size_t i = 0; i < stack->Controls.length; i++)
	{
		Control *c = ListGetPointer(stack->Controls, i);

		c->anchoredPosition = CalculateControlPosition(c);
	}

	if (IsMouseButtonPressed(SDL_BUTTON_LEFT) || IsButtonPressed(CONTROLLER_OK))
	{
		SetFocusedControl(stack, stack->ActiveControl);
		//stack->focusedControl = stack->ActiveControl;
		stack->ActiveControlState = ACTIVE;
		return stack->ActiveControl != -1;
	}

	stack->ActiveControl = -1;
	stack->ActiveControlState = NORMAL;

	if (UseController())
	{
		stack->ActiveControl = stack->focusedControl;
		stack->ActiveControlState = HOVER;
		if (IsButtonPressed(CONTROLLER_OK))
		{
			stack->ActiveControlState = ACTIVE;
		}
	} else
	{
		// iterate through the controls in reverse order so that the last control is on top and gets priority
		for (int i = (int)stack->Controls.length - 1; i >= 0; i--)
		{
			const Control *c = (Control *)ListGetPointer(stack->Controls, i);

			const Vector2 localMousePos = v2(mousePos.x - c->anchoredPosition.x, mousePos.y - c->anchoredPosition.y);
			if (localMousePos.x >= 0 &&
				localMousePos.x <= c->size.x &&
				localMousePos.y >= 0 &&
				localMousePos.y <= c->size.y)
			{
				stack->ActiveControl = i;
				if (IsMouseButtonPressed(SDL_BUTTON_LEFT) ||
					IsKeyJustPressed(SDL_SCANCODE_SPACE) ||
					IsButtonJustPressed(CONTROLLER_OK))
				{
					stack->ActiveControlState = ACTIVE;
					// make this control the focused control
					SetFocusedControl(stack, i);
				} else
				{
					stack->ActiveControlState = HOVER;
				}
				break;
			}
		}
	}

	// process tab and shift+tab to cycle through controls
	if ((IsKeyJustPressed(SDL_SCANCODE_TAB) && !IsKeyPressed(SDL_SCANCODE_LSHIFT)) ||
		IsButtonJustPressed(SDL_CONTROLLER_BUTTON_DPAD_DOWN))
	{
		if (stack->focusedControl == -1)
		{
			SetFocusedControl(stack, 0);
		} else
		{
			SetFocusedControl(stack, (int)((stack->focusedControl + 1) % stack->Controls.length));
		}
	} else if ((IsKeyJustPressed(SDL_SCANCODE_TAB) && IsKeyPressed(SDL_SCANCODE_LSHIFT)) ||
			   IsButtonJustPressed(SDL_CONTROLLER_BUTTON_DPAD_UP))
	{
		if (stack->focusedControl == -1)
		{
			SetFocusedControl(stack, (int)stack->Controls.length - 1);
		} else
		{
			const int fc = wrapi(stack->focusedControl - 1, 0, stack->Controls.length);
			SetFocusedControl(stack, fc);
		}
		// ensure the index is positive
		if (stack->focusedControl < 0)
		{
			SetFocusedControl(stack, stack->focusedControl + (int)stack->Controls.length);
		}
	}


	// return whether the mouse is over a control
	return stack->ActiveControl != -1;
}

void SetFocusedControl(UiStack *stack, const int index)
{
	if (stack->Controls.length == 0)
	{
		return;
	}
	if (stack->focusedControl == index)
	{
		return;
	}

	if (stack->focusedControl != -1)
	{
		const Control *c = ListGetPointer(stack->Controls, stack->focusedControl);
		if (ControlUnfocusFuncs[c->type] != NULL)
		{
			ControlUnfocusFuncs[c->type](c);
		}
	}

	stack->focusedControl = index;

	if (stack->focusedControl != -1)
	{
		const Control *c = ListGetPointer(stack->Controls, stack->focusedControl);
		if (ControlFocusFuncs[c->type] != NULL)
		{
			ControlFocusFuncs[c->type](c);
		}
	}
}

void DrawUiStack(const UiStack *stack)
{
	for (int i = 0; i < stack->Controls.length; i++)
	{
		const Control *c = ListGetPointer(stack->Controls, i);
		ControlDrawFuncs[c->type](c,
								  i == stack->ActiveControl ? stack->ActiveControlState : NORMAL,
								  c->anchoredPosition);

		// if this is the focused control, draw a border around it
		if (i == stack->focusedControl)
		{
			DrawNinePatchTexture(v2(c->anchoredPosition.x - 4, c->anchoredPosition.y - 4),
								 v2(c->size.x + 8, c->size.y + 8),
								 16,
								 16,
								 TEXTURE("interface/focus_rect"));
		}
	}
}

Vector2 CalculateControlPosition(const Control *control)
{
	Vector2 pos = control->position;
	const ControlAnchor anchor = control->anchor;

	switch (anchor)
	{
		case TOP_LEFT:
			pos = v2(0, 0);
			break;
		case TOP_CENTER:
			pos.x = (WindowWidthFloat() - control->size.x) / 2;
			pos.y = 0;
			break;
		case TOP_RIGHT:
			pos.x = WindowWidthFloat() - control->size.x;
			pos.y = 0;
			break;
		case MIDDLE_LEFT:
			pos.y = (WindowHeightFloat() - control->size.y) / 2;
			pos.x = 0;
			break;
		case MIDDLE_CENTER:
			pos = v2((WindowWidthFloat() - control->size.x) / 2, (WindowHeightFloat() - control->size.y) / 2);
			break;
		case MIDDLE_RIGHT:
			pos = v2(WindowWidthFloat() - control->size.x, (WindowHeightFloat() - control->size.y) / 2);
			break;
		case BOTTOM_LEFT:
			pos.y = WindowHeightFloat() - control->size.y;
			pos.x = 0;
			break;
		case BOTTOM_CENTER:
			pos = v2((WindowWidthFloat() - control->size.x) / 2, WindowHeightFloat() - control->size.y);
			break;
		case BOTTOM_RIGHT:
			pos = v2(WindowWidthFloat() - control->size.x, WindowHeightFloat() - control->size.y);
			break;
	}

	pos = Vector2Add(pos, control->position);

	return pos;
}

Control *CreateEmptyControl()
{
	Control *c = malloc(sizeof(Control));
	CheckAlloc(c);
	c->ControlData = NULL;
	return c;
}

void UiStackPush(UiStack *stack, Control *control)
{
	ListAdd(stack->Controls, control);
}

void UiStackRemove(UiStack *stack, const Control *control)
{
	ControlDestroyFuncs[control->type](control);

	ListRemoveAt(stack->Controls, ListFind(stack->Controls, control));
}

bool IsMouseInRect(const Vector2 pos, const Vector2 size)
{
	const Vector2 mousePos = GetMousePos();
	return mousePos.x >= pos.x && mousePos.x <= pos.x + size.x && mousePos.y >= pos.y && mousePos.y <= pos.y + size.y;
}

bool HasMouseActivation(UiStack * /*stack*/, const Control *Control)
{
	return IsMouseInRect(Control->anchoredPosition, Control->size) && IsMouseButtonJustReleased(SDL_BUTTON_LEFT);
}

bool HasKeyboardActivation(UiStack * /*stack*/, Control * /*Control*/)
{
	return IsKeyJustPressed(SDL_SCANCODE_RETURN) ||
		   IsKeyJustPressed(SDL_SCANCODE_SPACE) ||
		   IsButtonJustReleased(CONTROLLER_OK);
}

bool HasActivation(UiStack *stack, Control *Control)
{
	const size_t index = ListFind(stack->Controls, Control);
	bool focus = false;
	if (index == stack->ActiveControl)
	{
		focus |= HasMouseActivation(stack, Control);
	}
	if (index == stack->focusedControl)
	{
		focus |= HasKeyboardActivation(stack, Control);
	}
	return focus;
}

void UiStackResetFocus(UiStack *stack)
{
	SetFocusedControl(stack, UseController() ? 0 : -1);
}
