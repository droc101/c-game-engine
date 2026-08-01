//
// Created by droc101 on 7/31/26.
//

#ifndef GAME_SCROLLVIEW_H
#define GAME_SCROLLVIEW_H

#include <engine/structs/List.h>
#include <engine/structs/Vector2.h>
#include <engine/uiStack/controls/VScrollBar.h>
#include <engine/uiStack/UiStack.h>

typedef struct ScrollView
{
	UiStack *stack;

	ControlAnchor anchor;
	Vector2 position;
	Vector2 size;
	Vector2 anchoredPosition;

	List children;
	Control *scrollBar;

	VScrollBarData scrollBarData;
} ScrollView;

ScrollView *CreateScrollView(UiStack *stack, ControlAnchor anchor, Vector2 position, Vector2 size);
void FreeScrollView(ScrollView *view);

void ScrollViewAddChild(ScrollView *view, Control *control);

void ProcessScrollView(ScrollView *view);

Vector2 CalculateScrollViewScrollBarPosition(const Control *control, const void *positioningData);

Vector2 CalculateScrollViewChildPosition(const Control *control, const void *positioningData);

#endif //GAME_SCROLLVIEW_H
