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

static int MeasureLine(const char *str, const int line)
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
	if (start >= end)
	{
		out[0] = '\0';
		return;
	}

	size_t copySize = end - start;
	if (copySize >= outBufferSize)
	{
		copySize = outBufferSize - 1;
	}

	strncpy(out, str + start, copySize);
	out[copySize] = '\0';

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
	float *verts = malloc(stringLength * sizeof(float[4][4]));
	CheckAlloc(verts);
	uint32_t *indices = malloc(stringLength * sizeof(uint32_t[6]));
	CheckAlloc(indices);
	BatchedQuadArray quads;
	quads.verts = verts;
	quads.indices = indices;
	quads.quadCount = (int)stringLength;

	const double sizeMultiplier = (double)size / font->defaultSize;
	const float width = (float)(font->width * sizeMultiplier);
	const float quadHeight = (float)(font->textureHeight * sizeMultiplier);
	int c = 0;

	int lines = 1;
	for (size_t i = 0; i < stringLength; i++)
	{
		if (str[i] == '\n')
		{
			lines++;
			quads.quadCount--;
		}
	}
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
		float lx = (float)x;
		const float ly = (float)y;
		for (size_t j = 0; line[j] != '\0'; j++)
		{
			const int fSize = (int)((font->charWidths[(int)line[j]] + font->charSpacing) * sizeMultiplier);

			if (line[j] == ' ')
			{
				lx += (float)((font->spaceWidth + font->charSpacing) * sizeMultiplier);
				quads.quadCount--;
				c--;
				continue;
			}

			const Vector2 ndcPos = v2(X_TO_NDC(lx), Y_TO_NDC(ly));
			const Vector2 ndcPosEnd = v2(X_TO_NDC(lx + width), Y_TO_NDC(ly + quadHeight));
			const float charUVStart = font->charStartUVs[(int)line[j]];
			const float charUVEnd = font->charEndUVs[(int)line[j]];

			const size_t charIndex = c + j;
			const size_t vertexOffset = charIndex * 4;
			const size_t indexOffset = charIndex * 6;

			// *vert++ is used for optimization reasons (thanks compiler...)
			float *vert = verts + vertexOffset * 4;
			*vert++ = ndcPos.x;
			*vert++ = ndcPos.y;
			*vert++ = charUVStart;
			*vert++ = 0;
			*vert++ = ndcPos.x;
			*vert++ = ndcPosEnd.y;
			*vert++ = charUVStart;
			*vert++ = 1;
			*vert++ = ndcPosEnd.x;
			*vert++ = ndcPosEnd.y;
			*vert++ = charUVEnd;
			*vert++ = 1;
			*vert++ = ndcPosEnd.x;
			*vert++ = ndcPos.y;
			*vert++ = charUVEnd;
			*vert = 0;

			indices[indexOffset + 0] = 0;
			indices[indexOffset + 1] = 1;
			indices[indexOffset + 2] = 2;
			indices[indexOffset + 3] = 0;
			indices[indexOffset + 4] = 2;
			indices[indexOffset + 5] = 3;
			for (int k = 0; k < 6; k++)
			{
				indices[indexOffset + k] += vertexOffset;
			}

			lx += fSize;
		}
		c += (int)strlen(line);
		y += (int)(size + font->lineSpacing);
	}

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
