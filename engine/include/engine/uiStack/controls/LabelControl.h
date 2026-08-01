//
// Created by droc101 on 7/31/26.
//

#ifndef GAME_LABELCONTROL_H
#define GAME_LABELCONTROL_H

#include <engine/assets/FontLoader.h>
#include <engine/graphics/Font.h>
#include <engine/structs/Color.h>
#include <engine/structs/Vector2.h>
#include <engine/uiStack/UiStack.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct LabelControlData LabelControlData;

struct LabelControlData
{
	char *str;
	uint32_t size;
	Color color;
	FontHorizontalAlign hAlign;
	FontVerticalAlign vAlign;
	const Font *font;
	bool shadow;
};

Control *CreateLabelControl(char *str,
							uint32_t size,
							Color color,
							Vector2 rectPos,
							Vector2 rectSize,
							ControlAnchor anchor,
							FontHorizontalAlign hAlign,
							FontVerticalAlign vAlign,
							const Font *font,
							bool shadow);

void DestroyLabelControl(const Control *c);

void DrawLabelControl(const Control *c, ControlState state, Vector2 position);

#endif //GAME_LABELCONTROL_H
