//
// Created by droc101 on 8/21/26.
//

#include "gameState/AddonsState.h"
#include <engine/assets/AddonLoader.h>
#include <engine/assets/AssetReader.h>
#include <engine/graphics/Font.h>
#include <engine/graphics/RenderingHelpers.h>
#include <engine/helpers/BackgroundMapManager.h>
#include <engine/structs/Color.h>
#include <engine/structs/GameState.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/List.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Discord.h>
#include <engine/subsystem/Input.h>
#include <engine/uiStack/controls/Button.h>
#include <engine/uiStack/controls/Image.h>
#include <engine/uiStack/controls/LabelControl.h>
#include <engine/uiStack/UiStack.h>
#include <SDL3/SDL_scancode.h>
#include <stdbool.h>
#include <stddef.h>
#include <time.h>
#include "gameState/MenuState.h"
#include "helpers/OptionsMenu.h"

static OptionsMenu *addonsOptionsMenu = NULL;

static void ApplyAddons()
{
	ApplyAddonAssetPaths();
	ChangeMap(NULL);
	EnterMenuBackgroundState();
	rendererQueuedActions |= QUEUED_ACTION_RELOAD_ALL_ASSETS;
}

static void DoneButton(Control *, void *)
{
	ApplyAddons();
	SetGameState(&MenuState);
}

static void AddonsStateUpdate(GlobalState *state, const double delta)
{
	if (IsKeyJustPressed(mainThreadInput, SDL_SCANCODE_ESCAPE) ||
		IsButtonJustPressed(mainThreadInput, CONTROLLER_CANCEL))
	{
		DoneButton(NULL, NULL);
	}
	UpdateMenuBackground(state, delta);
}

static void AddonsStateRender(GlobalState *state, const double /*delta*/)
{
	RenderMenuBackground(state, true);
	if (!IsBackgroundMapLoaded())
	{
		return;
	}

	ProcessOptionsMenu(addonsOptionsMenu);
}

static void AddonsStateSet()
{
	GetState()->rpcState = IN_MENUS;
	if (addonsOptionsMenu == NULL)
	{
		addonsOptionsMenu = CreateOptionsMenu();

		if (addons.length == 0)
		{
			OptionsMenuAddControl(addonsOptionsMenu,
								  CreateLabelControl("No Addons Found",
													 16,
													 COLOR_WHITE,
													 v2(0, 0),
													 v2(750, 32),
													 TOP_CENTER,
													 FONT_HALIGN_CENTER,
													 FONT_VALIGN_MIDDLE,
													 FONT("small_font"),
													 false));
		}

		for (size_t i = 0; i < addons.length; i++)
		{
			Addon *a = ListGetPointer(addons, i);
			char *iconName = GetAddonIcon(a);
			Control *icon = CreateImageControl(v2(0, 0), v2s(96), iconName, TOP_LEFT, NULL);
			OptionsMenuAddControl(addonsOptionsMenu, icon);
			OptionsMenuAddControl(addonsOptionsMenu,
								  CreateLabelControl(a->displayName,
													 16,
													 COLOR_WHITE,
													 v2(104, 0),
													 v2(750 - 104, 32),
													 TOP_LEFT,
													 FONT_HALIGN_LEFT,
													 FONT_VALIGN_MIDDLE,
													 FONT("small_font"),
													 false));
			OptionsMenuAddControl(addonsOptionsMenu,
								  CreateLabelControl(a->description,
													 16,
													 COLOR(0xFFEEEEEE),
													 v2(104, 40),
													 v2(750 - 104, 96 - 40),
													 TOP_LEFT,
													 FONT_HALIGN_LEFT,
													 FONT_VALIGN_TOP,
													 FONT("small_font"),
													 false));

			OptionsMenuNextRow(addonsOptionsMenu);
		}

		OptionsMenuAddNoButtonHeaderFooter(addonsOptionsMenu, "Addons");
		UiStackPush(addonsOptionsMenu->stack,
					CreateButtonControl(v2(0, -40), v2(480, 40), "Done", DoneButton, BOTTOM_CENTER, NULL));
	}
	UiStackResetFocus(addonsOptionsMenu->stack);
	EnterMenuBackgroundState();
}

static void AddonsStateDestroy()
{
	if (addonsOptionsMenu != NULL)
	{
		DestroyOptionsMenu(addonsOptionsMenu);
		addonsOptionsMenu = NULL;
	}
}

const GameState AddonsState = {
	.UpdateGame = AddonsStateUpdate,
	.RenderGame = AddonsStateRender,
	.FixedUpdateGame = FixedUpdateMenuBackground,
	.Destroy = AddonsStateDestroy,
	.Set = AddonsStateSet,
	.enableRelativeMouseMode = false,
};
