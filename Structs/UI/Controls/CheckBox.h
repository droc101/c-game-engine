//
// Created by droc101 on 10/27/2024.
//

#ifndef GAME_CHECKBOX_H
#define GAME_CHECKBOX_H

#include <stdbool.h>
#include <stdint.h>
#include "../../Vector2.h"
#include "../UiStack.h"

typedef struct CheckBoxData CheckBoxData;

typedef void (*CheckboxCallback)(bool checked);

struct CheckBoxData
{
	char *label;
	bool checked;
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
 * @return The new Checkbox Control
 */
Control *CreateCheckboxControl(Vector2 position,
							   Vector2 size,
							   char *label,
							   CheckboxCallback callback,
							   ControlAnchor anchor,
							   bool checked);

void DestroyCheckbox(const Control *c);

void UpdateCheckbox(UiStack *stack, Control *c, Vector2 localMousePos, uint32_t ctlIndex);

void DrawCheckbox(const Control *c, ControlState state, Vector2 position);

#endif //GAME_CHECKBOX_H
