//
// Created by droc101 on 7/20/26.
//

#ifndef GAME_VSCROLLBAR_H
#define GAME_VSCROLLBAR_H
#include <engine/structs/Vector2.h>
#include <engine/uiStack/UiStack.h>
#include <stdint.h>

typedef struct VScrollBarData VScrollBarData;

struct VScrollBarData
{
	int scrollPos;
	int contentHeight;
};

Control *CreateVScrollBarControl(Vector2 position, float height, ControlAnchor anchor, VScrollBarData *scrollBarData);

void DestroyVScrollBar(const Control *c);

void UpdateVScrollBar(UiStack *stack, Control *c, Vector2 localMousePos, uint32_t ctlIndex);

void DrawVScrollBar(const Control *c, ControlState state, Vector2 position);

#endif //GAME_VSCROLLBAR_H
