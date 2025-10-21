//
// Created by droc101 on 10/7/2024.
//

#include <engine/uiStack/UiStack.h>
#include <SDL_gamecontroller.h>
#include <SDL_mouse.h>
#include <SDL_scancode.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <engine/assets/AssetReader.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Input.h>
#include <engine/structs/List.h>
#include <engine/graphics/Drawing.h>
#include <engine/graphics/RenderingHelpers.h>
#include <engine/structs/Vector2.h>

#include <engine/uiStack/controls/Button.h>
#include <engine/uiStack/controls/CheckBox.h>
#include <engine/uiStack/controls/RadioButton.h>
#include <engine/uiStack/controls/Slider.h>
#include <engine/uiStack/controls/TextBox.h>

typedef void (*ControlDrawFunc)(const Control *, ControlState state, Vector2 position);
typedef void (*ControlUpdateFunc)(UiStack *stack, Control *, Vector2 localMousePos, uint32_t ctlIndex);
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
	ListInit(stack->controls, LIST_POINTER);
	stack->activeControl = -1u;
	stack->activeControlState = NORMAL;
	stack->focusedControl = -1u;
	UiStackResetFocus(stack);
	return stack;
}

void DestroyUiStack(UiStack *stack)
{
	for (size_t i = 0; i < stack->controls.length; i++)
	{
		const Control *c = ListGetPointer(stack->controls, i);
		ControlDestroyFuncs[c->type](c);
	}
	ListAndContentsFree(stack->controls);
	free(stack);
	stack = NULL;
}

bool ProcessUiStack(UiStack *stack)
{
	const Vector2 mousePos = GetMousePos();

	if (stack->focusedControl != -1u)
	{
		Control *c = ListGetPointer(stack->controls, stack->focusedControl);
		ControlUpdateFuncs[c->type](stack,
									c,
									v2(mousePos.x - c->position.x, mousePos.y - c->position.y),
									stack->focusedControl);
	}
	if (stack->activeControl != -1u)
	{
		Control *c = ListGetPointer(stack->controls, stack->activeControl);
		ControlUpdateFuncs[c->type](stack,
									c,
									v2(mousePos.x - c->position.x, mousePos.y - c->position.y),
									stack->activeControl);
	}


	for (size_t i = 0; i < stack->controls.length; i++)
	{
		Control *c = ListGetPointer(stack->controls, i);

		c->anchoredPosition = CalculateControlPosition(c);
	}

	if (IsMouseButtonPressed(SDL_BUTTON_LEFT) || IsButtonPressed(CONTROLLER_OK))
	{
		SetFocusedControl(stack, stack->activeControl);
		//stack->focusedControl = stack->activeControl;
		stack->activeControlState = ACTIVE;
		return stack->activeControl != -1u;
	}

	stack->activeControl = -1;
	stack->activeControlState = NORMAL;

	if (UseController())
	{
		stack->activeControl = stack->focusedControl;
		stack->activeControlState = HOVER;
		if (IsButtonPressed(CONTROLLER_OK))
		{
			stack->activeControlState = ACTIVE;
		}
	} else
	{
		// iterate through the controls in reverse order so that the last control is on top and gets priority
		for (int i = (int)stack->controls.length - 1; i >= 0; i--)
		{
			const Control *c = (Control *)ListGetPointer(stack->controls, i);

			const Vector2 localMousePos = v2(mousePos.x - c->anchoredPosition.x, mousePos.y - c->anchoredPosition.y);
			if (localMousePos.x >= 0 &&
				localMousePos.x <= c->size.x &&
				localMousePos.y >= 0 &&
				localMousePos.y <= c->size.y)
			{
				stack->activeControl = i;
				if (IsMouseButtonPressed(SDL_BUTTON_LEFT) ||
					IsKeyJustPressed(SDL_SCANCODE_SPACE) ||
					IsButtonJustPressed(CONTROLLER_OK))
				{
					stack->activeControlState = ACTIVE;
					// make this control the focused control
					SetFocusedControl(stack, i);
				} else
				{
					stack->activeControlState = HOVER;
				}
				break;
			}
		}
	}

	// process tab and shift+tab to cycle through controls
	if ((IsKeyJustPressed(SDL_SCANCODE_TAB) && !IsKeyPressed(SDL_SCANCODE_LSHIFT)) ||
		IsButtonJustPressed(SDL_CONTROLLER_BUTTON_DPAD_DOWN))
	{
		if (stack->focusedControl == -1u)
		{
			SetFocusedControl(stack, 0);
		} else
		{
			SetFocusedControl(stack, (stack->focusedControl + 1) % stack->controls.length);
		}
	} else if ((IsKeyJustPressed(SDL_SCANCODE_TAB) && IsKeyPressed(SDL_SCANCODE_LSHIFT)) ||
			   IsButtonJustPressed(SDL_CONTROLLER_BUTTON_DPAD_UP))
	{
		if (stack->focusedControl == -1u || stack->focusedControl == 0)
		{
			SetFocusedControl(stack, stack->controls.length - 1);
		} else
		{
			SetFocusedControl(stack, (stack->focusedControl - 1) % stack->controls.length);
		}
	}


	// return whether the mouse is over a control
	return stack->activeControl != -1u;
}

void SetFocusedControl(UiStack *stack, const uint32_t index)
{
	if (stack->controls.length == 0)
	{
		return;
	}
	if (stack->focusedControl == index)
	{
		return;
	}

	if (stack->focusedControl != -1u)
	{
		const Control *c = ListGetPointer(stack->controls, stack->focusedControl);
		if (ControlUnfocusFuncs[c->type] != NULL)
		{
			ControlUnfocusFuncs[c->type](c);
		}
	}

	stack->focusedControl = index;

	if (stack->focusedControl != -1u)
	{
		const Control *c = ListGetPointer(stack->controls, stack->focusedControl);
		if (ControlFocusFuncs[c->type] != NULL)
		{
			ControlFocusFuncs[c->type](c);
		}
	}
}

void DrawUiStack(const UiStack *stack)
{
	for (size_t i = 0; i < stack->controls.length; i++)
	{
		const Control *c = ListGetPointer(stack->controls, i);
		ControlDrawFuncs[c->type](c,
								  i == stack->activeControl ? stack->activeControlState : NORMAL,
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
	c->controlData = NULL;
	return c;
}

void UiStackPush(UiStack *stack, Control *control)
{
	ListAdd(stack->controls, control);
}

void UiStackRemove(UiStack *stack, const Control *control)
{
	ControlDestroyFuncs[control->type](control);

	ListRemoveAt(stack->controls, ListFind(stack->controls, control));
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
	const size_t index = ListFind(stack->controls, Control);
	bool focus = false;
	if (index == stack->activeControl)
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
