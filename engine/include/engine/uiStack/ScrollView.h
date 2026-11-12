//
// Created by droc101 on 7/31/26.
//

/**
 * @file ScrollView.h
 * Vertically scrolling container for controls
 */

#ifndef GAME_SCROLLVIEW_H
#define GAME_SCROLLVIEW_H

#include <engine/structs/List.h>
#include <engine/structs/Vector2.h>
#include <engine/uiStack/controls/VScrollBar.h>
#include <engine/uiStack/UiStack.h>

/// A vertically scrolling region for UiStack controls
typedef struct ScrollView
{
	/// The UiStack this ScrollView belongs to
	UiStack *stack;

	/// The positioning anchor for this ScrollView
	ControlAnchor anchor;
	/// The position of this ScrollView
	Vector2 position;
	/// The size of this ScrollView
	Vector2 size;
	/// The anchor-calculated position of this ScrollView
	Vector2 anchoredPosition;

	/// The controls contained in this ScrollView
	List children;
	/// The VScrollBar controlling this ScrollView
	Control *scrollBar;

	/// Scroll bar data
	VScrollBarData scrollBarData;
} ScrollView;

/**
 * Create a ScrollView
 * @param stack The UiStack the ScrollView belongs to
 * @param anchor The positioning anchor
 * @param position The position of the ScrollView
 * @param size The size of the ScrollView
 */
ScrollView *CreateScrollView(UiStack *stack, ControlAnchor anchor, Vector2 position, Vector2 size);

/**
 * Destroy a scroll view
 * @param view The ScrollView to destroy
 */
void FreeScrollView(ScrollView *view);

/**
 * Add a control to a ScrollView
 * @param view The ScrollView to add to
 * @param control The Control to add
 */
void ScrollViewAddChild(ScrollView *view, Control *control);

/**
 * Process a ScrollView
 * @param view The ScrollView to process
 */
void ProcessScrollView(ScrollView *view);

/// @private
Vector2 CalculateScrollViewScrollBarPosition(const Control *control, const void *positioningData);

/// @private
Vector2 CalculateScrollViewChildPosition(const Control *control, const void *positioningData);

#endif //GAME_SCROLLVIEW_H
