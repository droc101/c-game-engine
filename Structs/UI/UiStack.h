//
// Created by droc101 on 10/7/2024.
//

#ifndef GAME_UISTACK_H
#define GAME_UISTACK_H

#include "../../defines.h"

typedef enum ControlType ControlType;
typedef enum ControlState ControlState;
typedef enum ControlAnchor ControlAnchor;

typedef struct Control Control;
typedef struct UiStack UiStack;

enum ControlType
{
	BUTTON,
	SLIDER,
	CHECKBOX,
	RADIO_BUTTON,
	TEXTBOX,
	CONTROL_TYPE_COUNT
};

enum ControlState
{
	NORMAL,
	HOVER,
	ACTIVE
};

enum ControlAnchor
{
	TOP_LEFT,
	TOP_CENTER,
	TOP_RIGHT,
	MIDDLE_LEFT,
	MIDDLE_CENTER,
	MIDDLE_RIGHT,
	BOTTOM_LEFT,
	BOTTOM_CENTER,
	BOTTOM_RIGHT
};

struct Control
{
	/// The type of the control
	ControlType type;
	/// The anchor of the control
	ControlAnchor anchor;
	/// The position relative to the anchor
	Vector2 position;
	/// The size of the control
	Vector2 size;

	/// The absolute position of the control, calculated automatically
	Vector2 anchoredPosition;

	/// Extra data for the control
	void *ControlData;
};

struct UiStack
{
	/// The controls in the UiStack
	List controls;

	/// The control that is currently being hovered or pressed
	uint activeControl;
	/// The state of the active control
	ControlState activeControlState;

	/// The control that has keyboard focus
	uint focusedControl;
};

/**
 * Create a new UiStack
 * @return The new UiStack
 */
UiStack *CreateUiStack();

/**
 * Destroy a UiStack
 * @param stack The UiStack to destroy
 */
void DestroyUiStack(UiStack *stack);

/**
 * Process the UiStack
 * @param stack The UiStack to process
 * @return Whether the mouse is over a control
 */
bool ProcessUiStack(UiStack *stack);

/**
 * Draw the UiStack
 * @warning Call @c ProcessUiStack before calling this
 * @param stack The UiStack to draw
 */
void DrawUiStack(const UiStack *stack);

/**
 * Calculate the position of a control based on its anchor
 * @param control The control to calculate the position for
 * @return The anchored position of the control
 */
Vector2 CalculateControlPosition(const Control *control);

/**
 * Create an empty control.
 * @warning This control should not be used directly, use the Create*Control functions instead
 * @return The empty control
 */
Control *CreateEmptyControl();

/**
 * Add a control to the UiStack
 * @param stack The UiStack to add the control to
 * @param control The control to add
 */
void UiStackPush(UiStack *stack, Control *control);

/**
 * Remove a control from the UiStack
 * @param stack The UiStack to remove the control from
 * @param control The control to remove
 */
void UiStackRemove(UiStack *stack, const Control *control);

/**
 * Check if the mouse is in a rectangle
 * @param pos The position of the rectangle
 * @param size The size of the rectangle
 * @return Whether the mouse is in the rectangle
 */
bool IsMouseInRect(Vector2 pos, Vector2 size);

/**
 * Check if a control is activated (keyboard or mouse)
 * @param stack The UiStack to check
 * @param Control The control to check
 * @return Whether the control is activated
 */
bool HasActivation(UiStack *stack, Control *Control);

/**
 * Reset the focus of a UiStack (to either the first control or none, depending on whether controller is used)
 * @param stack The UiStack to reset the focus of
 */
void UiStackResetFocus(UiStack *stack);

/**
 * Set the focused control index and call the focus/unfocus functions if necessary
* @param stack The UiStack to set the focused control of
 * @param index The index of the control to focus
 */
void SetFocusedControl(UiStack *stack, uint index);

#endif //GAME_UISTACK_H
