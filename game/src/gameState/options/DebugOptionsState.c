//
// Created by droc101 on 7/20/26.
//

#include "gameState/options/DebugOptionsState.h"
#include <engine/assets/AssetReader.h>
#include <engine/debug/DebugEntryManager.h>
#include <engine/graphics/Drawing.h>
#include <engine/graphics/Font.h>
#include <engine/helpers/BackgroundMapManager.h>
#include <engine/structs/Color.h>
#include <engine/structs/GameState.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/List.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Input.h>
#include <engine/uiStack/controls/LabelControl.h>
#include <engine/uiStack/controls/OptionsButton.h>
#include <engine/uiStack/UiStack.h>
#include <gameState/OptionsState.h>
#include <helpers/OptionsMenu.h>
#include <SDL3/SDL_scancode.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

static OptionsMenu *debugOptionsMenu;
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
		RenderMenuBackground(state, true);
	}

	ProcessOptionsMenu(debugOptionsMenu);
}

static void DebugOptionsStateSet()
{
	if (debugOptionsMenu == NULL)
	{
		debugOptionsMenu = CreateOptionsMenu();

		for (size_t i = 0; i < debugEntries.length; i++)
		{
			DebugEntry *entry = ListGetPointer(debugEntries, i);
			OptionsMenuAddControl(debugOptionsMenu,
								  CreateLabelControl(entry->key,
													 16,
													 COLOR_WHITE,
													 v2(0, 0),
													 v2(750 - 220 - 6, 40),
													 TOP_LEFT,
													 FONT_HALIGN_LEFT,
													 FONT_VALIGN_MIDDLE,
													 FONT("small_font"),
													 true));
			OptionsMenuAddControl(debugOptionsMenu,
								  CreateOptionsButtonControl(v2(0, 0),
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
			OptionsMenuNextRow(debugOptionsMenu);
		}

		OptionsMenuAddTwoButtonHeaderFooter(debugOptionsMenu,
											"Debug Options",
											"Reset to Defaults",
											BtnDebugOptionsReset,
											"Back",
											BtnDebugOptionsBack);

		// UiStackPush(debugOptionsStack,
		// 			CreateTextBoxControl("Filter Options",
		// 								 v2(0, 50),
		// 								 v2(480, 40),
		// 								 TOP_CENTER,
		// 								 64,
		// 								 FilterTextChanged,
		// 								 NULL));
	}
	UiStackResetFocus(debugOptionsMenu->stack);
}

static void DebugOptionsStateDestroy()
{
	if (debugOptionsMenu != NULL)
	{
		DestroyOptionsMenu(debugOptionsMenu);
		debugOptionsMenu = NULL;
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
