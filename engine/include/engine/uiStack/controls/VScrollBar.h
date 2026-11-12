//
// Created by droc101 on 7/20/26.
//

/**
 * @file VScrollBar.h
 * Vertical scroll bar
 */

#ifndef GAME_VSCROLLBAR_H
#define GAME_VSCROLLBAR_H
#include <engine/structs/Vector2.h>
#include <engine/uiStack/UiStack.h>
#include <stdint.h>

typedef struct VScrollBarData VScrollBarData;

/// Scroll bar data
struct VScrollBarData
{
	/// The current scroll position
	int scrollPos;
	/// The height of the content to scroll through
	int contentHeight;
	/// @private
	float dragPos;
};

/**
 * Create a vertical scroll bar control
 * @param position The position of the scroll bar
 * @param height The height of the scroll bar
 * @param anchor The positioning anchor
 * @param scrollBarData The backing data
 */
Control *CreateVScrollBarControl(Vector2 position, float height, ControlAnchor anchor, VScrollBarData *scrollBarData);

/// @private
void DestroyVScrollBar(const Control *c);

/// @private
void UpdateVScrollBar(UiStack *stack, Control *c, Vector2 localMousePos, uint32_t ctlIndex);

/// @private
void AlwaysUpdateVScrollBar(UiStack *stack, Control *c, Vector2 localMousePos, uint32_t ctlIndex);

/// @private
void DrawVScrollBar(const Control *c, ControlState state, Vector2 position);

#endif //GAME_VSCROLLBAR_H
