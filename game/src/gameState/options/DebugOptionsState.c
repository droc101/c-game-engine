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
#include <engine/helpers/MathEx.h>
#include <engine/structs/Color.h>
#include <engine/structs/GameState.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/List.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Input.h>
#include <engine/subsystem/SoundSystem.h>
#include <engine/uiStack/controls/Button.h>
#include <engine/uiStack/controls/TextBox.h>
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
static int scrollY = 0;
static char *filter = NULL;

static const int LIST_WIDTH = 650;
static const int ENTRY_HEIGHT = 44;

static void BtnDebugOptionsBack()
{
	SetGameState(&OptionsState);
}

static void BtnDebugOptionsReset()
{
	for (size_t i = 0; i < debugEntries.length; i++)
	{
		DebugEntry *entry = ListGetPointer(debugEntries, i);
		entry->enabled = entry->defaultEnabled;
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
	}

	const int listX = ScaledWindowWidth() / 2 - LIST_WIDTH / 2;

	int entryY = 102 + scrollY;
	int numShownEntries = 0;
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

		const Vector2 buttonPos = v2(listX + LIST_WIDTH - 200, entryY + 2);
		const Vector2 buttonSize = v2(200, ENTRY_HEIGHT - 4);

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
				entry->enabled = !entry->enabled;
				SaveDebugEntrySettings();
				UiStackResetFocus(debugOptionsStack);
				(void)PlaySound(SOUND("sfx/click"), SOUND_CATEGORY_UI);
			}
		}

		DrawNinePatchTexture(buttonPos, buttonSize, 8, 8, buttonTexture);
		DrawTextAligned(entry->enabled ? "Enabled" : "Disabled",
						16,
						entry->enabled ? COLOR(0xFF008000) : COLOR(0xFF800000),
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
	DrawLine(v2(0, 100), v2(ScaledWindowWidth(), 99), 2, COLOR_WHITE);
	DrawLine(v2(0, ScaledWindowHeight() - 100), v2(ScaledWindowWidth(), ScaledWindowHeight() - 99), 2, COLOR_WHITE);

	DrawTextAligned("Debug Options",
					32,
					COLOR_WHITE,
					v2s(0),
					v2(ScaledWindowWidthFloat(), 50),
					FONT_HALIGN_CENTER,
					FONT_VALIGN_MIDDLE,
					largeFont);

	ProcessUiStack(debugOptionsStack);
	DrawUiStack(debugOptionsStack);

	const int allEntriesHeight = numShownEntries * (ENTRY_HEIGHT + 2);
	const int listAreaHeight = ScaledWindowHeight() - 200;

	if (allEntriesHeight > listAreaHeight)
	{
		const int SCROLLBAR_WIDTH = 16;

		DrawRect((ScaledWindowWidth() / 2 + LIST_WIDTH / 2) + 4,
				 100,
				 SCROLLBAR_WIDTH,
				 ScaledWindowHeight() - 200,
				 COLOR_BLACK);

		const float percentVisible = (1.0f / (float)allEntriesHeight) * (float)listAreaHeight;
		const float percentScrolled = (1.0f / (float)allEntriesHeight) * (float)abs(scrollY);

		const Vector2 scrollbarPos = v2((ScaledWindowWidthFloat() / 2 + (float)LIST_WIDTH / 2) + 5,
										100 + listAreaHeight * percentScrolled);
		const Vector2 scrollbarSize = v2(SCROLLBAR_WIDTH - 2, listAreaHeight * percentVisible);

		Color scrollbarGrabberColor = COLOR_WHITE;
		if (IsRectHovered(scrollbarPos, scrollbarSize))
		{
			if (IsMouseButtonPressed(mainThreadInput, SDL_BUTTON_LEFT))
			{
				scrollY -= (int)GetMouseRel(mainThreadInput).y;
				scrollbarGrabberColor = COLOR(0xFFc9daf8);
			} else
			{
				scrollbarGrabberColor = COLOR(0xFFf0f0ff);
			}
		}
		DrawRect((int)scrollbarPos.x,
				 (int)scrollbarPos.y,
				 (int)scrollbarSize.x,
				 (int)scrollbarSize.y,
				 scrollbarGrabberColor);
	}

	scrollY += (int)GetMouseWheelTicks(mainThreadInput).y * ENTRY_HEIGHT / 4;
	if (allEntriesHeight > listAreaHeight)
	{
		scrollY = clamp(scrollY, -(allEntriesHeight - listAreaHeight + 2), 0);
	} else
	{
		scrollY = 0;
	}
}

static void DebugOptionsStateSet()
{
	if (debugOptionsStack == NULL)
	{
		debugOptionsStack = CreateUiStack();

		UiStackPush(debugOptionsStack,
					CreateTextBoxControl("Filter Options", v2(0, 50), v2(450, 40), TOP_CENTER, 64, FilterTextChanged));

		UiStackPush(debugOptionsStack,
					CreateButtonControl(v2(-175, -40),
										v2(340, 40),
										"Reset to Defaults",
										BtnDebugOptionsReset,
										BOTTOM_CENTER));
		UiStackPush(debugOptionsStack,
					CreateButtonControl(v2(175, -40), v2(340, 40), "Back", BtnDebugOptionsBack, BOTTOM_CENTER));
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
