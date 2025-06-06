//
// Created by droc101 on 7/11/2024.
//

#include "TextBox.h"
#include "CommonAssets.h"
#include "Graphics/Drawing.h"
#include "Graphics/Font.h"
#include "Graphics/RenderingHelpers.h"

#define BOX_OUTER_PADDING 14
#define TEXT_BOX_FONT_SIZE 24
#define TEXT_BOX_FONT_WIDTH 18

typedef struct TextBoxTheme TextBoxTheme;

struct TextBoxTheme
{
	Color boxColor;
	Color textColor;
};

TextBoxTheme textBoxThemes[3] = {
	{COLOR(0x80000000), COLOR_WHITE},
	{COLOR(0xA0FFFFFF), COLOR_BLACK},
	{COLOR(0x80200000), COLOR(0xFFFFEEEE)},
};

void TextBoxRender(const TextBox *box, const int page)
{
	const int startLine = box->rows * page;
	const int lineCount = StringLineCount(box->text);
	int endLine = startLine + box->rows;
	if (endLine > lineCount)
	{
		endLine = lineCount;
	}

	Vector2 topLeft = {0, 0};

	const Color textColor = textBoxThemes[box->theme].textColor;
	const Color boxColor = textBoxThemes[box->theme].boxColor;

	const int width = box->cols * TEXT_BOX_FONT_WIDTH + BOX_OUTER_PADDING * 2;
	const int height = box->rows * TEXT_BOX_FONT_SIZE + BOX_OUTER_PADDING * 2;

	if (box->hAlign == TEXT_BOX_H_ALIGN_CENTER)
	{
		topLeft.x = (WindowWidthFloat() - (float)width) / 2; // NOLINT(*-integer-division)
	} else if (box->hAlign == TEXT_BOX_H_ALIGN_RIGHT)
	{
		topLeft.x = WindowWidthFloat() - (float)width;
	} else
	{
		topLeft.x = 0;
	}

	if (box->vAlign == TEXT_BOX_V_ALIGN_CENTER)
	{
		topLeft.y = (WindowHeightFloat() - (float)height) / 2; // NOLINT(*-integer-division)
	} else if (box->vAlign == TEXT_BOX_V_ALIGN_BOTTOM)
	{
		topLeft.y = WindowHeightFloat() - (float)height;
	} else
	{
		topLeft.y = 0;
	}

	topLeft.x += (float)box->x;
	topLeft.y += (float)box->y;

	DrawRect((int)topLeft.x,
			 (int)topLeft.y,
			 box->cols * TEXT_BOX_FONT_WIDTH + BOX_OUTER_PADDING * 2,
			 box->rows * TEXT_BOX_FONT_SIZE + BOX_OUTER_PADDING * 2,
			 boxColor);

	int txtY = (int)topLeft.y + BOX_OUTER_PADDING;
	for (int i = startLine; i < endLine; i++)
	{
		char line[256];
		TextGetLine(box->text, i, line, 256);
		const Vector2 pos = {topLeft.x + BOX_OUTER_PADDING, (float)txtY};
		FontDrawString(pos, line, TEXT_BOX_FONT_SIZE, textColor, smallFont);
		txtY += TEXT_BOX_FONT_SIZE;
	}
}
