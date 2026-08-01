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

typedef void (*OptionsButtonCallback)(size_t value, void *extraData);

struct OptionsButtonValue
{
	char *text;
	char *tooltip;
	uint64_t value;
};

struct OptionsButtonData
{
	char *text;
	OptionsButtonCallback callback;
	bool enabled;
	OptionsButtonValue *values;
	size_t numValues;
	size_t value;
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
									size_t value);

void DestroyOptionsButton(const Control *c);

void UpdateOptionsButton(UiStack *stack, Control *c, Vector2 localMousePos, uint32_t ctlIndex);

void DrawOptionsButton(const Control *c, ControlState state, Vector2 position);

#endif //GAME_OPTIONSBUTTON_H
