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
#include <engine/uiStack/controls/Button.h>
#include <engine/uiStack/controls/HeaderFooterControl.h>
#include <engine/uiStack/controls/LabelControl.h>
#include <engine/uiStack/controls/OptionsButton.h>
#include <engine/uiStack/ScrollView.h>
#include <engine/uiStack/UiStack.h>
#include <gameState/OptionsState.h>
#include <SDL3/SDL_scancode.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

static UiStack *debugOptionsStack = NULL;
static ScrollView *debugOptionsScrollView = NULL;
static char *filter = NULL;

static OptionsButtonValue buttonValues[3] = {
	{
		.text = "Disabled",
		.tooltip = "This debug option will never be shown",
		.value =
				{
					.type = CONTROL_VALUE_DWORD,
					.dwordValue = DEBUG_ENTRY_DISABLED,
				},
	},
	{
		.text = "In Extended Menu",
		.tooltip = "This debug option will be shown in the extended menu",
		.value =
				{
					.type = CONTROL_VALUE_DWORD,
					.dwordValue = DEBUG_ENTRY_TOGGLE,
				},
	},
	{
		.text = "Always Visible",
		.tooltip = "This debug option will always be shown",
		.value =
				{
					.type = CONTROL_VALUE_DWORD,
					.dwordValue = DEBUG_ENTRY_SHOWN,
				},
	},
};

static const int ENTRY_HEIGHT = 44;

static void BtnDebugOptionsBack(Control *, void *)
{
	SetGameState(&OptionsState);
}

static void BtnDebugOptionsReset(Control *, void *)
{
	for (size_t i = 0; i < debugEntries.length; i++)
	{
		DebugEntry *entry = ListGetPointer(debugEntries, i);
		entry->mode = entry->defaultMode;
		SetGameState(&DebugOptionsState);
	}
}

static void OptBtnEntryChanged(const OptionsButtonValue *value, void *extraData)
{
	DebugEntry *entry = extraData;
	entry->mode = value->value.dwordValue;
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
		BtnDebugOptionsBack(NULL, NULL);
	}
	if (!optionsStateInGame)
	{
		UpdateMenuBackground(state, delta);
	}
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

	debugOptionsScrollView->size.y = ScaledWindowHeightFloat() - 200;
	ProcessScrollView(debugOptionsScrollView);
	ProcessUiStack(debugOptionsStack);
	DrawUiStack(debugOptionsStack);
}

static void DebugOptionsStateSet()
{
	if (debugOptionsStack == NULL)
	{
		debugOptionsStack = CreateUiStack();
		debugOptionsScrollView = CreateScrollView(debugOptionsStack, TOP_CENTER, v2(0, 100), v2(750, 200));

		int entryY = 0;
		for (size_t i = 0; i < debugEntries.length; i++)
		{
			DebugEntry *entry = ListGetPointer(debugEntries, i);
			ScrollViewAddChild(debugOptionsScrollView,
							   CreateLabelControl(entry->key,
												  16,
												  COLOR_WHITE,
												  v2(0, entryY),
												  v2(750 - 220 - 6, 40),
												  TOP_LEFT,
												  FONT_HALIGN_LEFT,
												  FONT_VALIGN_MIDDLE,
												  FONT("small_font"),
												  true));
			ScrollViewAddChild(debugOptionsScrollView,
							   CreateOptionsButtonControl(v2(0, entryY),
														  v2(220, 40),
														  "%s",
														  OptBtnEntryChanged,
														  TOP_RIGHT,
														  buttonValues,
														  DEBUG_ENTRY_MODE_MAX,
														  entry,
														  (ControlValue){
															  .type = CONTROL_VALUE_DWORD,
															  .dwordValue = &entry->mode,
														  },
														  NULL));

			entryY += ENTRY_HEIGHT;
		}

		UiStackPush(debugOptionsStack, CreateHeaderFooterControl(100, true, "Debug Options"));
		UiStackPush(debugOptionsStack, CreateHeaderFooterControl(100, false, NULL));

		// UiStackPush(debugOptionsStack,
		// 			CreateTextBoxControl("Filter Options",
		// 								 v2(0, 50),
		// 								 v2(480, 40),
		// 								 TOP_CENTER,
		// 								 64,
		// 								 FilterTextChanged,
		// 								 NULL));

		UiStackPush(debugOptionsStack,
					CreateButtonControl(v2(-175, -40),
										v2(340, 40),
										"Reset to Defaults",
										BtnDebugOptionsReset,
										BOTTOM_CENTER,
										NULL));
		UiStackPush(debugOptionsStack,
					CreateButtonControl(v2(175, -40), v2(340, 40), "Back", BtnDebugOptionsBack, BOTTOM_CENTER, NULL));
	}
	UiStackResetFocus(debugOptionsStack);
}

static void DebugOptionsStateDestroy()
{
	if (debugOptionsStack != NULL)
	{
		FreeScrollView(debugOptionsScrollView);
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
