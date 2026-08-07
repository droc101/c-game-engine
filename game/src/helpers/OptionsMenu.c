//
// Created by droc101 on 8/7/26.
//

#include "helpers/OptionsMenu.h"
#include <engine/assets/AssetReader.h>
#include <engine/graphics/Font.h>
#include <engine/graphics/RenderingHelpers.h>
#include <engine/structs/Color.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Error.h>
#include <engine/uiStack/controls/Button.h>
#include <engine/uiStack/controls/HeaderFooterControl.h>
#include <engine/uiStack/controls/LabelControl.h>
#include <engine/uiStack/ScrollView.h>
#include <engine/uiStack/UiStack.h>
#include <stdbool.h>
#include <stdlib.h>

#define OPTION_VERTICAL_SPACING 45
#define OPTION_SECTION_SPACING 24

OptionsMenu *CreateOptionsMenu()
{
	OptionsMenu *builder = malloc(sizeof(OptionsMenu));
	CheckAlloc(builder);
	builder->stack = CreateUiStack();
	builder->scrollView = CreateScrollView(builder->stack, TOP_CENTER, v2(0, 100), v2(750, 200));
	builder->smallOptionRightSide = false;
	builder->yPos = 0;
	return builder;
}

void OptionsMenuAddSection(OptionsMenu *menu, char *label)
{
	if (menu->smallOptionRightSide)
	{
		menu->yPos += OPTION_VERTICAL_SPACING;
		menu->smallOptionRightSide = false;
	}
	menu->yPos += OPTION_SECTION_SPACING;
	if (label)
	{
		Control *labelControl = CreateLabelControl(label,
												   16,
												   COLOR_WHITE,
												   v2(0, menu->yPos),
												   v2(750, 40),
												   TOP_CENTER,
												   FONT_HALIGN_LEFT,
												   FONT_VALIGN_MIDDLE,
												   FONT("small_font"),
												   true);
		OptionsMenuAddLargeControl(menu, labelControl);
	}
}

void OptionsMenuAddLargeControl(OptionsMenu *menu, Control *control)
{
	control->size = v2(750, 40);
	control->position = v2(0, menu->yPos);
	control->anchor = TOP_CENTER;
	ScrollViewAddChild(menu->scrollView, control);
	menu->yPos += OPTION_VERTICAL_SPACING;
	menu->smallOptionRightSide = false;
}

void OptionsMenuAddSmallControl(OptionsMenu *menu, Control *control)
{
	control->size = v2(370, 40);
	control->anchor = TOP_CENTER;
	if (menu->smallOptionRightSide)
	{
		control->position = v2(190, menu->yPos);
	} else
	{
		control->position = v2(-190, menu->yPos);
	}
	ScrollViewAddChild(menu->scrollView, control);
	if (menu->smallOptionRightSide)
	{
		menu->yPos += OPTION_VERTICAL_SPACING;
	}
	menu->smallOptionRightSide = !menu->smallOptionRightSide;
}

void OptionsMenuAddHeaderFooter(OptionsMenu *menu, char *title, const ButtonCallback doneButtonCallback)
{
	UiStackPush(menu->stack, CreateHeaderFooterControl(100, true, title));
	UiStackPush(menu->stack, CreateHeaderFooterControl(100, false, NULL));
	if (doneButtonCallback)
	{
		UiStackPush(menu->stack,
					CreateButtonControl(v2(0, -40), v2(480, 40), "Back", doneButtonCallback, BOTTOM_CENTER, NULL));
	}
}

void ProcessOptionsMenu(OptionsMenu *menu)
{
	menu->scrollView->size.y = ScaledWindowHeightFloat() - 200;
	ProcessScrollView(menu->scrollView);
	ProcessUiStack(menu->stack);
	DrawUiStack(menu->stack);
}

void DestroyOptionsMenu(OptionsMenu *menu)
{
	FreeScrollView(menu->scrollView);
	DestroyUiStack(menu->stack);
	free(menu);
}
