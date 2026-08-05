//
// Created by droc101 on 7/31/26.
//

#ifndef GAME_OPTIONSBUTTON_H
#define GAME_OPTIONSBUTTON_H

#include <engine/structs/Vector2.h>
#include <engine/uiStack/UiStack.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct OptionsButtonData OptionsButtonData;
typedef struct OptionsButtonValue OptionsButtonValue;

typedef void (*OptionsButtonCallback)(const OptionsButtonValue *value, void *extraData);

struct OptionsButtonValue
{
	char *text;
	char *tooltip;
	LiteralControlValue value;
};

struct OptionsButtonData
{
	char *text;
	char *alwaysTooltip;
	OptionsButtonCallback callback;
	bool enabled;
	OptionsButtonValue *values;
	size_t numValues;
	ControlValue value;
	void *callbackExtraData;
};

Control *CreateOptionsButtonControl(Vector2 position,
									Vector2 size,
									char *text,
									OptionsButtonCallback callback,
									ControlAnchor anchor,
									OptionsButtonValue *values,
									size_t numValues,
									void *extraData,
									ControlValue value,
									char *alwaysTooltip);

void DestroyOptionsButton(const Control *c);

void UpdateOptionsButton(UiStack *stack, Control *c, Vector2 localMousePos, uint32_t ctlIndex);

void DrawOptionsButton(const Control *c, ControlState state, Vector2 position);

extern OptionsButtonValue onOffButtonValues[2];
extern OptionsButtonValue yesNoButtonValues[2];

#endif //GAME_OPTIONSBUTTON_H
