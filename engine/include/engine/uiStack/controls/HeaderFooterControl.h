//
// Created by droc101 on 7/31/26.
//

/**
 * @file HeaderFooterControl.h
 * Menu screen header or footer control
 */

#ifndef GAME_HEADERCONTROL_H
#define GAME_HEADERCONTROL_H

#include <engine/structs/Vector2.h>
#include <engine/uiStack/UiStack.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct HeaderFooterControlData HeaderFooterControlData;

/// @private
struct HeaderFooterControlData
{
	/// Whether this is a header or footer
	bool isHeader;
	/// The text to show on the header
	char *label;
};

/**
 * Create a HeaderFooter control
 * @param height The height of this control
 * @param isHeader Whether this should be a header or a footer
 * @param label The label to show on the header
 */
Control *CreateHeaderFooterControl(float height, bool isHeader, char *label);

/// @private
void DestroyHeaderFooterControl(const Control *c);

/// @private
void AlwaysUpdateHeaderFooterControl(UiStack *stack, Control *c, Vector2 localMousePos, uint32_t ctlIndex);

/// @private
void DrawHeaderFooterControl(const Control *c, ControlState state, Vector2 position);

#endif //GAME_HEADERCONTROL_H
