//
// Created by droc101 on 4/26/2024.
//

#include "DPrint.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include "../config.h"
#include "../Helpers/CommonAssets.h"
#include "../Helpers/Core/Logging.h"
#include "../Helpers/Graphics/Font.h"
#include "../Structs/Color.h"
#include "../Structs/Vector2.h"

int dprintYPosition = 10;

void ResetDPrintYPos()
{
	dprintYPosition = 10;
}

void DPrint(const char *str, const Color color)
{
#ifdef ENABLE_DEBUG_PRINT
	FontDrawString(v2(12, (float)dprintYPosition + 2), str, 16, COLOR_BLACK, smallFont);
	FontDrawString(v2(10, (float)dprintYPosition), str, 16, color, smallFont);
	dprintYPosition += (int)MeasureText(str, 16, smallFont).y + 8;
#endif
}

void DPrintF(const char *str, const Color color, const bool con, ...)
{
#ifdef ENABLE_DEBUG_PRINT
	char buffer[256];
	va_list args;
	va_start(args, con);
	vsprintf(buffer, str, args);
	va_end(args);
	DPrint(buffer, color);
	if (con)
	{
		LogInfo(buffer);
	}
#endif
}
