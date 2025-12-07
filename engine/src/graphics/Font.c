//
// Created by droc101 on 4/21/2024.
//

#include <cglm/types.h>
#include <ctype.h>
#include <engine/assets/AssetReader.h>
#include <engine/assets/FontLoader.h>
#include <engine/graphics/Drawing.h>
#include <engine/graphics/Font.h>
#include <engine/graphics/RenderingHelpers.h>
#include <engine/helpers/MathEx.h>
#include <engine/structs/Color.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Logging.h>
#include <float.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

Font *smallFont;
Font *largeFont;

inline void FontDrawString(const Vector2 pos, const char *str, const uint32_t size, const Color color, const Font *font)
{
	DrawTextAligned(str, size, color, pos, v2s(FLT_MAX), FONT_HALIGN_LEFT, FONT_VALIGN_TOP, font);
}

inline Vector2 MeasureText(const char *str, const uint32_t size, const Font *font)
{
	return MeasureTextNChars(str, size, font, strlen(str));
}

Vector2 MeasureTextNChars(const char *str, const uint32_t size, const Font *font, const size_t n)
{
	int textWidth = 0;
	int textHeight = (int)size;
	int tempWidth = 0;
	const double sizeMultiplier = (double)size / font->defaultSize;
	for (size_t j = 0; j < n; j++)
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
	const size_t length = strlen(str);
	for (size_t i = 0; i < length; i++)
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

void TextGetLine(const char *str, const int line, char *out, const size_t outBufferSize, const bool convertToUppercase)
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

	if (convertToUppercase)
	{
		for (size_t i = 0; i < copySize; i++)
		{
			out[i] = (char)toupper(out[i]);
		}
	}
}

void DrawTextAligned(const char *str,
					 const uint32_t size,
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
	uint32_t *indices = calloc(stringLength, sizeof(uint32_t[6]));
	CheckAlloc(indices);

	const double sizeMultiplier = (double)size / font->defaultSize;
	const int width = (int)(font->width * sizeMultiplier);
	const int quadHeight = (int)(font->textureHeight * sizeMultiplier);
	const double uvPixel = 1.0 / (double)font->image->width;
	int c = 0;

	const int lines = StringLineCount(str);
	int x = 0;
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
		TextGetLine(str, i, line, 256, font->uppercaseOnly);
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

			uint32_t quadIndices[6] = {0, 1, 2, 0, 2, 3};
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

void InitCommonFonts()
{
	LogDebug("Loading fonts...\n");
	smallFont = LoadFont(FONT("small_font"));
	largeFont = LoadFont(FONT("large_font"));
}

void DestroyCommonFonts()
{
	LogDebug("Cleaning up fonts...\n");
	FreeFont(smallFont);
	FreeFont(largeFont);
}
