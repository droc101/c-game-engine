//
// Created by droc101 on 7/31/26.
//

#include <assert.h>
#include <engine/debug/DebugEntryManager.h>
#include <engine/graphics/Drawing.h>
#include <engine/structs/List.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Input.h>
#include <engine/uiStack/controls/VScrollBar.h>
#include <engine/uiStack/ScrollView.h>
#include <engine/uiStack/UiStack.h>
#include <stddef.h>
#include <stdlib.h>

ScrollView *CreateScrollView(UiStack *stack, const ControlAnchor anchor, const Vector2 position, const Vector2 size)
{
	ScrollView *view = malloc(sizeof(ScrollView));
	CheckAlloc(view);

	view->stack = stack;

	view->anchor = anchor;
	view->position = position;
	view->size = size;

	ListInit(view->children, LIST_POINTER);
	view->scrollBar = CreateVScrollBarControl(v2(position.x + (size.x / 2), position.y),
											  size.y,
											  anchor,
											  &view->scrollBarData);
	view->scrollBar->positioningData = view;
	view->scrollBar->CalculatePosition = CalculateScrollViewScrollBarPosition;
	UiStackPush(stack, view->scrollBar);

	view->scrollBarData.scrollPos = 0;
	view->scrollBarData.contentHeight = 0;

	return view;
}

void FreeScrollView(ScrollView *view)
{
	ListFree(view->children);
	free(view);
}

void ScrollViewAddChild(ScrollView *view, Control *control)
{
	assert(control->anchor == TOP_LEFT || control->anchor == TOP_CENTER || control->anchor == TOP_RIGHT);
	control->CalculatePosition = CalculateScrollViewChildPosition;
	control->positioningData = view;
	ListAdd(view->children, control);
	UiStackPush(view->stack, control);
}

void ProcessScrollView(ScrollView *view)
{
	float maxY = 0;
	for (size_t i = 0; i < view->children.length; i++)
	{
		const Control *c = ListGetPointer(view->children, i);
		const float bottom = c->position.y + c->size.y;
		if (bottom > maxY)
		{
			maxY = bottom;
		}
	}
	maxY += 32.0f;
	view->scrollBarData.contentHeight = (int)maxY;
	view->scrollBar->size.y = view->size.y;
	const Control imposter = {.position = view->position, .size = view->size, .anchor = view->anchor};
	view->anchoredPosition = CalculateControlPosition(&imposter, NULL);

	view->scrollBarData.scrollPos += (int)GetMouseWheelTicks(mainThreadInput).y * 20;

	if (IsDebugEntryVisible("ui_stack_layout_bounds"))
	{
		DrawRect(view->anchoredPosition.x, view->anchoredPosition.y, view->size.x, view->size.y, COLOR(0x800000ff));
	}
}

Vector2 CalculateScrollViewScrollBarPosition(const Control *control, const void *positioningData)
{
	const ScrollView *view = positioningData;
	return v2(view->anchoredPosition.x + view->size.x + 16, view->anchoredPosition.y);
}

Vector2 CalculateScrollViewChildPosition(const Control *control, const void *positioningData)
{
	const ScrollView *view = positioningData;
	const float viewLeft = view->anchoredPosition.x;
	const float viewHorizMiddle = viewLeft + (view->size.x / 2);
	const float viewRight = viewLeft + view->size.x;

	const float viewTop = view->anchoredPosition.y + 16 + (float)view->scrollBarData.scrollPos;

	Vector2 pos = control->position;
	const ControlAnchor anchor = control->anchor;

	switch (anchor)
	{
		case TOP_LEFT:
			pos = v2(viewLeft, viewTop);
			break;
		case TOP_CENTER:
			pos.x = viewHorizMiddle - (control->size.x / 2);
			pos.y = viewTop;
			break;
		case TOP_RIGHT:
			pos.x = viewRight - control->size.x;
			pos.y = viewTop;
			break;
		default:
			break;
	}

	pos = Vector2Add(pos, control->position);

	return pos;
}
