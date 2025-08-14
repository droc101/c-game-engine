//
// Created by droc101 on 10/27/2024.
//

#ifndef GAME_RADIOBUTTON_H
#define GAME_RADIOBUTTON_H

#include <stdbool.h>
#include <stdint.h>
#include "../../Vector2.h"
#include "../UiStack.h"

typedef struct RadioButtonData RadioButtonData;

typedef void (*RadioButtonCallback)(bool, uint8_t, uint8_t);

struct RadioButtonData
{
	char *label;
	uint8_t groupId;
	uint8_t id;
	bool checked;
	RadioButtonCallback callback;
};

/**
 * Create a new RadioButton Control
 * @param position The position of the RadioButton
 * @param size The size of the RadioButton
 * @param label The label of the RadioButton
 * @param callback The callback function to call when the RadioButton is checked
 * @param anchor The anchor of the RadioButton
 * @param checked Whether the RadioButton is checked or not
 * @param groupId The group id of the RadioButton. RadioButtons with the same group id will be mutually exclusive.
 * @param id The id of the RadioButton. This is passed to the callback function when the RadioButton is checked.
 * @return The new RadioButton Control
 */
Control *CreateRadioButtonControl(Vector2 position,
								  Vector2 size,
								  char *label,
								  RadioButtonCallback callback,
								  ControlAnchor anchor,
								  bool checked,
								  uint8_t groupId,
								  uint8_t id);

void DestroyRadioButton(const Control *c);

void UpdateRadioButton(UiStack *stack, Control *c, Vector2 localMousePos, uint32_t ctlIndex);

void DrawRadioButton(const Control *c, ControlState state, Vector2 position);

#endif //GAME_RADIOBUTTON_H
