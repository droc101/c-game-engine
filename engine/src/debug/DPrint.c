//
// Created by droc101 on 4/26/2024.
//

#include <engine/debug/DPrint.h>
#include <engine/graphics/Drawing.h>
#include <engine/graphics/Font.h>
#include <engine/structs/Color.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Logging.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

int dprintYPosition = 10;

void ResetDPrintYPos()
{
	dprintYPosition = 10;
}

void DPrint(const char *str, const Color color)
{
#ifdef ENABLE_DEBUG_PRINT
	const Vector2 textSize = MeasureText(str, 16, smallFont);
	DrawRect(5, dprintYPosition - 5, (int)textSize.x + 10, (int)textSize.y + 10, COLOR(0x80000000));
	FontDrawString(v2(10, (float)dprintYPosition), str, 16, color, smallFont);
	dprintYPosition += (int)textSize.y + 10;
#endif
}

void DPrintF(const char *format, const bool printToConsole, const Color color, ...)
{
#ifdef ENABLE_DEBUG_PRINT
	char buffer[256];
	va_list args;
	va_start(args, color);
	vsprintf(buffer, format, args);
	va_end(args);
	DPrint(buffer, color);
	if (printToConsole)
	{
		LogInfo(buffer);
	}
#endif
}
