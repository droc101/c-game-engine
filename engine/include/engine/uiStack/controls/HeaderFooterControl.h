//
// Created by droc101 on 7/31/26.
//

#ifndef GAME_HEADERCONTROL_H
#define GAME_HEADERCONTROL_H

#include <engine/structs/Vector2.h>
#include <engine/uiStack/UiStack.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct HeaderFooterControlData HeaderFooterControlData;

struct HeaderFooterControlData
{
	bool isHeader;
	char *label;
};

Control *CreateHeaderFooterControl(float height, bool isHeader, char *label);

void DestroyHeaderFooterControl(const Control *c);

void AlwaysUpdateHeaderFooterControl(UiStack *stack, Control *c, Vector2 localMousePos, uint32_t ctlIndex);

void DrawHeaderFooterControl(const Control *c, ControlState state, Vector2 position);

#endif //GAME_HEADERCONTROL_H
