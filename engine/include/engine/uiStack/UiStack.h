//
// Created by droc101 on 10/7/2024.
//

/**
 * @file UiStack.h
 * Basic GUI system
 */

#ifndef GAME_UISTACK_H
#define GAME_UISTACK_H

#include <engine/structs/List.h>
#include <engine/structs/Vector2.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct ControlValue ControlValue;
typedef struct LiteralControlValue LiteralControlValue;
typedef struct Control Control;
typedef struct UiStack UiStack;

/// @private
typedef enum ControlType
{
	BUTTON,
	SLIDER,
	CHECKBOX,
	RADIO_BUTTON,
	TEXTBOX,
	V_SCROLL_BAR,
	HEADER_FOOTER,
	LABEL,
	OPTIONS_BUTTON,
	ICON_BUTTON,
	IMAGE,
	CONTROL_TYPE_COUNT,
} ControlType;

/// @private
typedef enum ControlState
{
	NORMAL,
	HOVER,
	ACTIVE,
} ControlState;

/// Positioning anchor
typedef enum ControlAnchor
{
	TOP_LEFT,
	TOP_CENTER,
	TOP_RIGHT,
	MIDDLE_LEFT,
	MIDDLE_CENTER,
	MIDDLE_RIGHT,
	BOTTOM_LEFT,
	BOTTOM_CENTER,
	BOTTOM_RIGHT,
} ControlAnchor;

/// The type of data contained in an @c ControlValue
typedef enum ControlValueType
{
	CONTROL_VALUE_BOOL,
	CONTROL_VALUE_BYTE,
	CONTROL_VALUE_WORD,
	CONTROL_VALUE_DWORD,
	CONTROL_VALUE_FLOAT,
} ControlValueType;

///@privatesection
typedef void (*ControlDrawFunc)(const Control *, ControlState state, Vector2 position);
typedef void (*ControlUpdateFunc)(UiStack *stack, Control *, Vector2 localMousePos, uint32_t ctlIndex);
typedef void (*ControlDestroyFunc)(const Control *);
typedef void (*ControlFocusFunc)(const Control *);
typedef void (*ControlUnfocusFunc)(const Control *);
typedef Vector2 (*CalculateControlPositionFunc)(const Control *, const void *positioningData);
///@publicsection

/// A multi-type value a Control can modify
struct ControlValue
{
	/// The type of data contained in this ControlValue
	ControlValueType type;
	union
	{
		/// Boolean value pointer
		bool *boolValue;
		/// Byte value pointer
		uint8_t *byteValue;
		/// Word value pointer
		uint16_t *wordValue;
		/// Dword value pointer
		uint32_t *dwordValue;
		/// Float value pointer
		float *floatValue;
	};
};

/// A multi-type constant value for a Control
struct LiteralControlValue
{
	/// The type of data contained in this LiteralControlValue
	ControlValueType type;
	union
	{
		/// Boolean value
		bool boolValue;
		/// Byte value
		uint8_t byteValue;
		/// Word value
		uint16_t wordValue;
		/// Dword value
		uint32_t dwordValue;
		/// Float value
		float floatValue;
	};
};

/// A UiStack Control
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

	/// The tooltip of this control, or NULL for none
	char *tooltip;

	/// Extra data to use for positioning this control
	void *positioningData;

	/// Whether this control can be focused
	bool allowFocus;

	/// The absolute position of the control, calculated automatically
	Vector2 anchoredPosition;

	/// Extra data for the control
	void *controlData;

	/// @private
	ControlDestroyFunc DestroyCallback;
	/// @private
	ControlDrawFunc DrawCallback;
	/// @private
	ControlUpdateFunc UpdateCallback;
	/// @private
	ControlUpdateFunc AlwaysUpdateCallback;
	/// @private
	ControlFocusFunc FocusCallback;
	/// @private
	ControlUnfocusFunc UnfocusCallback;
	/// @private
	CalculateControlPositionFunc CalculatePosition;
};

/// A GUI screen
struct UiStack
{
	/// The controls in the UiStack
	List controls;

	/// The control that is currently being hovered or pressed
	uint32_t activeControl;
	/// The state of the active control
	ControlState activeControlState;

	/// The control that has keyboard focus
	uint32_t focusedControl;
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
 * @param positioningData Extra data for positioning, unused here
 * @return The anchored position of the control
 * @private
 */
Vector2 CalculateControlPosition(const Control *control, const void *positioningData);

/**
 * Create an empty control.
 * @warning This control should not be used directly, use the Create*Control functions instead
 * @return The empty control
 * @private
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
 * @private
 */
bool IsMouseInRect(Vector2 pos, Vector2 size);

/**
 * Check if a control is activated (keyboard or mouse)
 * @param stack The UiStack to check
 * @param Control The control to check
 * @return Whether the control is activated
 * @private
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
void SetFocusedControl(UiStack *stack, uint32_t index);

/**
 * Change the focused control for a UiStack
 * @param stack The UiStack to change focus for
 * @param by The amount to change by
 */
void ChangeFocusedControl(UiStack *stack, int32_t by);

/**
 * Render a tooltip at the mouse position
 * @param text The tooltip text
 */
void RenderTooltipAtMouse(const char *text);

/**
 * Render a tooltip at a specific position
 * @param text The tooltip text
 * @param origin The position to place the tooltip at
 */
void RenderTooltipAt(const char *text, Vector2 origin);

#endif //GAME_UISTACK_H
