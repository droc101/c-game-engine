//
// Created by droc101 on 4/21/2024.
//

#include "Font.h"
#include <string.h>
#include "../../Structs/Vector2.h"
#include "../Core/Error.h"
#include "../Core/MathEx.h"
#include "Drawing.h"
#include "RenderingHelpers.h"

inline void FontDrawString(const Vector2 pos, const char *str, const uint size, const Color color, const Font *font)
{
	DrawTextAligned(str, size, color, pos, v2s(FLT_MAX), FONT_HALIGN_LEFT, FONT_VALIGN_TOP, font);
}

inline Vector2 MeasureText(const char *str, const uint size, const Font *font)
{
	return MeasureTextNChars(str, size, font, strlen(str));
}

Vector2 MeasureTextNChars(const char *str, const uint size, const Font *font, const size_t n)
{
	int textWidth = 0;
	int textHeight = (int)size;
	int tempWidth = 0;
	const double sizeMultiplier = (double)size / font->defaultSize;
	for (int j = 0; j < n; j++)
	{
		const int fSize = (int)((font->charWidths[(int)str[j]] + font->charSpacing) * sizeMultiplier);
		tempWidth += fSize;
		if (str[j] == ' ')
		{
			tempWidth -= fSize;
			tempWidth += (int)((font->spaceWidth + font->charSpacing) * sizeMultiplier);
		} else if (str[j + 1] == '\0')
		{
			tempWidth -= (int)(font->charSpacing * sizeMultiplier); // fix extra spacing at the end of the string
		} else if (str[j] == '\n')
		{
			tempWidth -= fSize;
			textWidth = max(textWidth, tempWidth);
			tempWidth = 0;
			textHeight += (int)((size + font->lineSpacing) * sizeMultiplier);
		}
	}

	textWidth = max(textWidth, tempWidth);

	return v2((float)textWidth, (float)textHeight);
}

int StringLineCount(const char *str)
{
	int count = 1;
	for (int i = 0; i < strlen(str); i++)
	{
		if (str[i] == '\n')
		{
			count++;
		}
	}
	return count;
}

int MeasureLine(const char *str, const int line)
{
	int i = 0;
	int count = 0;
	while (str[i] != '\0')
	{
		if (str[i] == '\n')
		{
			count++;
		}
		if (count == line)
		{
			return i;
		}
		i++;
	}
	return i;
}

void TextGetLine(const char *str, const int line, char *out, size_t outBufferSize)
{
	int start = MeasureLine(str, line);

	// if the start is a newline, skip it
	if (str[start] == '\n')
	{
		start++;
	}

	const int end = MeasureLine(str, line + 1);

	// ensure start is less than end
	if (start > end)
	{
		out[0] = '\0';
		return;
	}

	size_t copySize = end - start;
	if (copySize > outBufferSize)
	{
		copySize = outBufferSize;
	}

	strncpy(out, str + start, copySize);
	out[end - start] = '\0';
}

void DrawTextAligned(const char *str,
					 const uint size,
					 const Color color,
					 const Vector2 rectPos,
					 const Vector2 rectSize,
					 const FontHorizontalAlign hAlign,
					 const FontVerticalAlign vAlign,
					 const Font *font)
{
	const size_t stringLength = strlen(str);
	float *verts = calloc(stringLength, sizeof(float[4][4]));
	CheckAlloc(verts);
	uint *indices = calloc(stringLength, sizeof(uint[6]));
	CheckAlloc(indices);

	const double sizeMultiplier = (double)size / font->defaultSize;
	const int width = (int)(font->width * sizeMultiplier);
	const int quadHeight = (int)(font->textureHeight * sizeMultiplier);
	const double uvPixel = 1.0 / font->image->width;
	int c = 0;

	const int lines = StringLineCount(str);
	int x;
	int y = (int)rectPos.y;
	if (vAlign == FONT_VALIGN_MIDDLE)
	{
		y += ((int)rectSize.y - lines * (int)size) / 2;
	} else if (vAlign == FONT_VALIGN_BOTTOM)
	{
		y += (int)rectSize.y - lines * (int)size;
	}

	for (int i = 0; i < lines; i++)
	{
		char line[256];
		TextGetLine(str, i, line, 256);
		const Vector2 textSize = MeasureText(line, size, font);
		if (hAlign == FONT_HALIGN_CENTER)
		{
			x = (int)(rectPos.x + (rectSize.x - textSize.x) / 2);
		} else if (hAlign == FONT_HALIGN_RIGHT)
		{
			x = (int)(rectPos.x + rectSize.x - textSize.x);
		} else
		{
			x = (int)rectPos.x;
		}
		int lx = x;
		const int ly = y;
		int j = 0;
		while (line[j] != '\0')
		{
			const int fSize = (int)((font->charWidths[(int)line[j]] + font->charSpacing) * sizeMultiplier);

			if (line[j] == ' ')
			{
				j++;
				lx += (int)((font->spaceWidth + font->charSpacing) * sizeMultiplier);
				continue;
			}

			const Vector2 ndcPos = v2(X_TO_NDC((float)lx), Y_TO_NDC((float)ly));
			const Vector2 ndcPosEnd = v2(X_TO_NDC((float)(lx + width)), Y_TO_NDC((float)(ly + quadHeight)));
			const double charUVStart = (double)font->indices[(int)line[j]] / font->charCount;
			const double charUVEnd = (font->indices[(int)line[j]] + 1.0) / font->charCount - uvPixel;

			const mat4 quad = {
				{(float)ndcPos.x, (float)ndcPos.y, (float)charUVStart, 0},
				{(float)ndcPos.x, (float)ndcPosEnd.y, (float)charUVStart, 1},
				{(float)ndcPosEnd.x, (float)ndcPosEnd.y, (float)charUVEnd, 1},
				{(float)ndcPosEnd.x, (float)ndcPos.y, (float)charUVEnd, 0},
			};

			memcpy(verts + (c + j) * 16, quad, sizeof(quad));

			uint quadIndices[6] = {0, 1, 2, 0, 2, 3};
			for (int k = 0; k < 6; k++)
			{
				quadIndices[k] += (c + j) * 4;
			}

			memcpy(indices + (c + j) * 6, quadIndices, sizeof(quadIndices));

			lx += fSize;
			j++;
		}
		c += (int)strlen(line);
		y += (int)(size + font->lineSpacing);
	}

	BatchedQuadArray quads;
	quads.verts = verts;
	quads.indices = indices;
	quads.quadCount = (int)stringLength;
	DrawBatchedQuadsTextured(&quads, font->texture, color);

	free(verts);
	free(indices);
}
