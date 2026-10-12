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
typedef struct OptionsButtonOptions OptionsButtonOptions;

typedef void (*OptionsButtonCallback)(const OptionsButtonValue *value, void *extraData);

struct OptionsButtonOptions
{
	size_t numValues;
	OptionsButtonValue *values;
};

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
	OptionsButtonOptions *options;
	ControlValue value;
	void *callbackExtraData;
};

Control *CreateOptionsButtonControl(Vector2 position,
									Vector2 size,
									char *text,
									OptionsButtonCallback callback,
									ControlAnchor anchor,
									OptionsButtonOptions *options,
									void *extraData,
									ControlValue value,
									char *alwaysTooltip);

void DestroyOptionsButton(const Control *c);

void UpdateOptionsButton(UiStack *stack, Control *c, Vector2 localMousePos, uint32_t ctlIndex);

void DrawOptionsButton(const Control *c, ControlState state, Vector2 position);

extern OptionsButtonOptions onOffButtonOptions;
extern OptionsButtonOptions yesNoButtonOptions;

#endif //GAME_OPTIONSBUTTON_H
