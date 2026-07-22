//
// Created by droc101 on 7/20/26.
//

#include "gameState/options/DebugOptionsState.h"
#include <engine/assets/AssetReader.h>
#include <engine/debug/DebugEntryManager.h>
#include <engine/graphics/Drawing.h>
#include <engine/graphics/Font.h>
#include <engine/graphics/RenderingHelpers.h>
#include <engine/helpers/BackgroundMapManager.h>
#include <engine/structs/Color.h>
#include <engine/structs/GameState.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/List.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Input.h>
#include <engine/subsystem/SoundSystem.h>
#include <engine/uiStack/controls/Button.h>
#include <engine/uiStack/controls/TextBox.h>
#include <engine/uiStack/controls/VScrollBar.h>
#include <engine/uiStack/UiStack.h>
#include <gameState/OptionsState.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_stdinc.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

static UiStack *debugOptionsStack = NULL;
static char *filter = NULL;

static const int LIST_WIDTH = 750;
static const int ENTRY_HEIGHT = 44;

static VScrollBarData scrollData = {0};

static void BtnDebugOptionsBack()
{
	SetGameState(&OptionsState);
}

static void BtnDebugOptionsReset()
{
	for (size_t i = 0; i < debugEntries.length; i++)
	{
		DebugEntry *entry = ListGetPointer(debugEntries, i);
		entry->mode = entry->defaultMode;
	}
}

static void FilterTextChanged(const char *newFilter)
{
	free(filter);
	filter = NULL;
	if (newFilter && strlen(newFilter) > 0)
	{
		filter = strdup(newFilter);
	}
}

static void DebugOptionsStateUpdate(GlobalState *state, const double delta)
{
	if (IsKeyJustPressed(mainThreadInput, SDL_SCANCODE_ESCAPE) ||
		IsButtonJustPressed(mainThreadInput, CONTROLLER_CANCEL))
	{
		BtnDebugOptionsBack();
	}
	if (!optionsStateInGame)
	{
		UpdateMenuBackground(state, delta);
	}
}

static bool IsRectHovered(Vector2 pos, Vector2 size)
{
	const Vector2 endPos = Vector2Add(pos, size);
	const Vector2 mousePos = GetMousePos(mainThreadInput);

	if (mousePos.y < 100 || mousePos.y > ScaledWindowHeightFloat() - 100)
	{
		return false;
	}

	return mousePos.x >= pos.x && mousePos.x <= endPos.x && mousePos.y > pos.y && mousePos.y <= endPos.y;
}

static void DebugOptionsStateRender(GlobalState *state, const double /*delta*/)
{
	if (optionsStateInGame)
	{
		RenderInGameMenuBackground();
	} else
	{
		RenderMenuBackground(state);
		DrawRect(0, 0, ScaledWindowWidth(), ScaledWindowHeight(), COLOR(0x40000000));
	}

	const int listX = ScaledWindowWidth() / 2 - LIST_WIDTH / 2;

	int entryY = 102 + scrollData.scrollPos;
	int numShownEntries = 0;
	bool showDebugEntryTooltip = false;
	DebugEntryMode tooltipMode = DEBUG_ENTRY_DISABLED;
	for (size_t i = 0; i < debugEntries.length; i++)
	{
		DebugEntry *entry = ListGetPointer(debugEntries, i);

		if (filter != NULL && strstr(entry->key, filter) == NULL)
		{
			continue;
		}

		DrawTextAligned(entry->key,
						20,
						COLOR_WHITE,
						v2(listX, entryY),
						v2(LIST_WIDTH, ENTRY_HEIGHT),
						FONT_HALIGN_LEFT,
						FONT_VALIGN_MIDDLE,
						smallFont);

		const Vector2 buttonPos = v2(listX + LIST_WIDTH - 220, entryY + 2);
		const Vector2 buttonSize = v2(220, ENTRY_HEIGHT - 4);

		const char *buttonTexture = TEXTURE("interface/button");
		if (IsRectHovered(buttonPos, buttonSize))
		{
			buttonTexture = TEXTURE("interface/button_hover");
			if (IsMouseButtonPressed(mainThreadInput, SDL_BUTTON_LEFT))
			{
				buttonTexture = TEXTURE("interface/button_pressed");
			}
			if (IsMouseButtonJustReleased(mainThreadInput, SDL_BUTTON_LEFT))
			{
				entry->mode++;
				if (entry->mode == DEBUG_ENTRY_MODE_MAX)
				{
					entry->mode = DEBUG_ENTRY_DISABLED;
				}
				SaveDebugEntrySettings();
				UiStackResetFocus(debugOptionsStack);
				(void)PlaySound(SOUND("sfx/click"), SOUND_CATEGORY_UI);
			}

			showDebugEntryTooltip = true;
			tooltipMode = entry->mode;
		}

		DrawNinePatchTexture(buttonPos, buttonSize, 8, 8, buttonTexture);
		const char *label = "Disabled";
		switch (entry->mode)
		{
			case DEBUG_ENTRY_TOGGLE:
				label = "In Extended Menu";
				break;
			case DEBUG_ENTRY_SHOWN:
				label = "Always Visible";
				break;
			default:
				break;
		}
		DrawTextAligned(label,
						16,
						COLOR_BLACK,
						buttonPos,
						buttonSize,
						FONT_HALIGN_CENTER,
						FONT_VALIGN_MIDDLE,
						smallFont);

		entryY += ENTRY_HEIGHT + 2;
		numShownEntries++;
	}

	DrawRect(0, 0, ScaledWindowWidth(), 100, COLOR(0x80000000));
	DrawRect(0, ScaledWindowHeight() - 100, ScaledWindowWidth(), 100, COLOR(0x80000000));
	DrawLine(v2(0, 99), v2(ScaledWindowWidth(), 99), 2, COLOR_WHITE);
	DrawLine(v2(0, ScaledWindowHeight() - 99), v2(ScaledWindowWidth(), ScaledWindowHeight() - 99), 2, COLOR_WHITE);

	DrawTextAligned("Debug Options",
					32,
					COLOR_WHITE,
					v2s(0),
					v2(ScaledWindowWidthFloat(), 50),
					FONT_HALIGN_CENTER,
					FONT_VALIGN_MIDDLE,
					largeFont);

	scrollData.contentHeight = numShownEntries * (ENTRY_HEIGHT + 2);
	scrollData.scrollPos += (int)GetMouseWheelTicks(mainThreadInput).y * ENTRY_HEIGHT / 2;

	ProcessUiStack(debugOptionsStack);
	DrawUiStack(debugOptionsStack);

	if (showDebugEntryTooltip)
	{
		switch (tooltipMode)
		{
			case DEBUG_ENTRY_DISABLED:
				RenderTooltipAtMouse("This debug option will never be shown");
				break;
			case DEBUG_ENTRY_TOGGLE:
				RenderTooltipAtMouse("This debug option will be shown in the extended menu");
				break;
			case DEBUG_ENTRY_SHOWN:
				RenderTooltipAtMouse("This debug option will always be shown");
				break;
			default:
				break;
		}
	}
}

static void DebugOptionsStateSet()
{
	if (debugOptionsStack == NULL)
	{
		debugOptionsStack = CreateUiStack();

		UiStackPush(debugOptionsStack,
					CreateTextBoxControl("Filter Options",
										 v2(0, 50),
										 v2(480, 40),
										 TOP_CENTER,
										 64,
										 FilterTextChanged,
										 NULL));

		UiStackPush(debugOptionsStack,
					CreateButtonControl(v2(-175, -40),
										v2(340, 40),
										"Reset to Defaults",
										BtnDebugOptionsReset,
										BOTTOM_CENTER,
										NULL));
		UiStackPush(debugOptionsStack,
					CreateButtonControl(v2(175, -40), v2(340, 40), "Back", BtnDebugOptionsBack, BOTTOM_CENTER, NULL));

		UiStackPush(debugOptionsStack,
					CreateVScrollBarControl(v2(LIST_WIDTH / 2 + 20, 0),
											ScaledWindowHeightFloat() - 200,
											MIDDLE_CENTER,
											&scrollData));
	}
	UiStackResetFocus(debugOptionsStack);
}

static void DebugOptionsStateDestroy()
{
	if (debugOptionsStack != NULL)
	{
		DestroyUiStack(debugOptionsStack);
		debugOptionsStack = NULL;
	}
}

const GameState DebugOptionsState = {
	.UpdateGame = DebugOptionsStateUpdate,
	.RenderGame = DebugOptionsStateRender,
	.FixedUpdateGame = OptionsStateFixedUpdate,
	.Destroy = DebugOptionsStateDestroy,
	.Set = DebugOptionsStateSet,
	.enableRelativeMouseMode = false,
};
