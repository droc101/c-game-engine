//
// Created by droc101 on 10/27/2024.
//

/**
 * @file CheckBox.h
 * Simple checkbox control
 */

#ifndef GAME_CHECKBOX_H
#define GAME_CHECKBOX_H

#include <engine/structs/Vector2.h>
#include <engine/uiStack/UiStack.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct CheckBoxData CheckBoxData;

/**
 * Checkbox checked callback signature
 * @param checked Whether the checkbox is checked
 */
typedef void (*CheckboxCallback)(bool checked);

/// @private
struct CheckBoxData
{
	/// The checkbox's label
	char *label;
	/// Whether the checkbox is checked
	bool checked;
	/// The function to call when the checkbox is checked/unchecked
	CheckboxCallback callback;
};

/**
 * Create a new Checkbox Control
 * @param position The position of the checkbox
 * @param size The size of the checkbox
 * @param label The label of the checkbox
 * @param callback The callback function to call when the checkbox is checked or unchecked
 * @param anchor The anchor of the checkbox
 * @param checked Whether the checkbox is checked or not
 * @param tooltip
 * @return The new Checkbox Control
 */
Control *CreateCheckboxControl(Vector2 position,
							   Vector2 size,
							   char *label,
							   CheckboxCallback callback,
							   ControlAnchor anchor,
							   bool checked,
							   char *tooltip);

/// @private
void DestroyCheckbox(const Control *c);

/// @private
void UpdateCheckbox(UiStack *stack, Control *c, Vector2 localMousePos, uint32_t ctlIndex);

/// @private
void DrawCheckbox(const Control *c, ControlState state, Vector2 position);

#endif //GAME_CHECKBOX_H
