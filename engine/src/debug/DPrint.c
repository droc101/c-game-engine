//
// Created by droc101 on 4/26/2024.
//

#include <engine/assets/AssetReader.h>
#include <engine/debug/DPrint.h>
#include <engine/graphics/Drawing.h>
#include <engine/graphics/Font.h>
#include <engine/graphics/RenderingHelpers.h>
#include <engine/structs/Color.h>
#include <engine/structs/Vector2.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

static int dprintLeftSideYPosition = 10;
static int dprintRightSideYPosition = 10;
static bool dprintIsRightSide = false;

void ResetDPrintYPos()
{
	dprintIsRightSide = false;
	dprintLeftSideYPosition = 10;
	dprintRightSideYPosition = 10;
}

static inline int *GetYPos()
{
	return dprintIsRightSide ? &dprintRightSideYPosition : &dprintLeftSideYPosition;
}

void DPrint(const char *str, const Color color)
{
	const Vector2 textSize = MeasureText(str, 16, FONT("small_font"));
	const int xpos = dprintIsRightSide ? (ScaledWindowWidth() - 20 - (int)textSize.x) : 5;
	DrawRect(xpos, *GetYPos() - 5, (int)textSize.x + 10, (int)textSize.y + 10, COLOR(0x80000000));
	FontDrawString(v2(xpos + 5, (float)*GetYPos()), str, 16, color, FONT("small_font"));
	*GetYPos() += (int)textSize.y + 10;
}

void DPrintF(const char *format, const Color color, ...)
{
	char buffer[256];
	va_list args;
	va_start(args, color);
	vsprintf(buffer, format, args);
	va_end(args);
	DPrint(buffer, color);
}

void DPrintSpacing(const uint32_t spacing)
{
	*GetYPos() += (int)spacing;
}

void DPrintSetSide(const bool isRightSide)
{
	dprintIsRightSide = isRightSide;
}
