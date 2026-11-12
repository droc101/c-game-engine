//
// Created by droc101 on 7/31/26.
//

/**
 * @file LabelControl.h
 * Simple text label control
 */

#ifndef GAME_LABELCONTROL_H
#define GAME_LABELCONTROL_H

#include <engine/graphics/Font.h>
#include <engine/structs/Vector2.h>
#include <engine/uiStack/UiStack.h>
#include <engine/uiStack/UiTheme.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct LabelControlData LabelControlData;

/// @private
struct LabelControlData
{
	const char *str;
	uint32_t size;
	UiTextColorset *colorset;
	FontHorizontalAlign hAlign;
	FontVerticalAlign vAlign;
	char *font;
	bool shadow;
};

/**
 * Create a new label control
 * @param str The text to show
 * @param size The size of the control
 * @param colorset The text colorset to use
 * @param rectPos The label position
 * @param rectSize The label size
 * @param anchor The positioning anchor
 * @param hAlign The horizontal text alignment
 * @param vAlign The vertical text alignment
 * @param font The font
 * @param shadow Whether to show a text shadow
 */
Control *CreateLabelControl(const char *str,
							uint32_t size,
							UiTextColorset *colorset,
							Vector2 rectPos,
							Vector2 rectSize,
							ControlAnchor anchor,
							FontHorizontalAlign hAlign,
							FontVerticalAlign vAlign,
							char *font,
							bool shadow);

/// @private
void DestroyLabelControl(const Control *c);

/// @private
void DrawLabelControl(const Control *c, ControlState state, Vector2 position);

#endif //GAME_LABELCONTROL_H
