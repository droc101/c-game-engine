//
// Created by droc101 on 8/21/26.
//

#include "gameState/AddonsState.h"
#include <assert.h>
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
static bool shouldRebuildUiStack = false;

static void ApplyAddons()
{
	ApplyAddonAssetPaths();
	ChangeMap(NULL);
	EnterMenuBackgroundState();
	rendererQueuedActions |= QUEUED_ACTION_RELOAD_ALL_ASSETS;
}

static void DoneButton(Control *, void *)
{
	SaveOptions(&GetState()->options);
	ApplyAddons();
	SetGameState(&MenuState);
}

static void EnableAddonBtn(Control *, void *pAddon)
{
	SetAddonEnabled(pAddon, true);
	shouldRebuildUiStack = true;
}

static void DisableAddonBtn(Control *, void *pAddon)
{
	SetAddonEnabled(pAddon, false);
	shouldRebuildUiStack = true;
}

static void Swap(List *list, const size_t a, const size_t b)
{
	void *first = ListGetPointer(*list, a);
	ListSet(*list, a, ListGetPointer(*list, b));
	ListSet(*list, b, first);
}

static void BtnAddonPriUp(Control *, void *addonIndexInAddress)
{
	const size_t index = (size_t)addonIndexInAddress;
	assert(index != 0);
	Swap(&enabledAddons, index, index - 1);
	shouldRebuildUiStack = true;
}

static void BtnAddonPriDown(Control *, void *addonIndexInAddress)
{
	const size_t index = (size_t)addonIndexInAddress;
	assert(index != enabledAddons.length - 1);
	Swap(&enabledAddons, index, index + 1);
	shouldRebuildUiStack = true;
}

static void ReconstructUiStack()
{
	shouldRebuildUiStack = false;
	if (addonsOptionsMenu != NULL)
	{
		DestroyOptionsMenu(addonsOptionsMenu);
	}
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
	} else
	{
		List disabledAddons;
		ListInit(disabledAddons, LIST_POINTER);

		for (size_t i = 0; i < addons.length; i++)
		{
			Addon *a = ListGetPointer(addons, i);
			if (!IsAddonEnabled(a))
			{
				ListAdd(disabledAddons, a);
			}
		}

		OptionsMenuAddSection(addonsOptionsMenu, "Enabled Addons");
		if (enabledAddons.length == 0)
		{
			OptionsMenuAddControl(addonsOptionsMenu,
								  CreateLabelControl("(none)",
													 16,
													 COLOR(0xFF808080),
													 v2(0, 0),
													 v2(750, 16),
													 TOP_CENTER,
													 FONT_HALIGN_LEFT,
													 FONT_VALIGN_MIDDLE,
													 FONT("small_font"),
													 true));
		}
		for (size_t i = 0; i < enabledAddons.length; i++)
		{
			Addon *a = GetAddonById(ListGetPointer(enabledAddons, i));
			const char *iconName = GetAddonIcon(a);
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
			Control *disableBtn = CreateButtonControl(v2(0, 28),
													  v2(120, 40),
													  "Disable",
													  DisableAddonBtn,
													  TOP_RIGHT,
													  NULL);
			((ButtonData *)disableBtn->controlData)->extraData = a;
			OptionsMenuAddControl(addonsOptionsMenu, disableBtn);

			if (i != 0)
			{
				Control *priUpBtn = CreateButtonControl(v2(-40, 0), v2(40, 28), "/\\", BtnAddonPriUp, TOP_RIGHT, NULL);
				((ButtonData *)priUpBtn->controlData)->extraData = (void *)i;
				OptionsMenuAddControl(addonsOptionsMenu, priUpBtn);
			}
			if (i != enabledAddons.length - 1)
			{
				Control *priDownBtn = CreateButtonControl(v2(-40, 68),
														  v2(40, 28),
														  "\\/",
														  BtnAddonPriDown,
														  TOP_RIGHT,
														  NULL);
				((ButtonData *)priDownBtn->controlData)->extraData = (void *)i;
				OptionsMenuAddControl(addonsOptionsMenu, priDownBtn);
			}

			OptionsMenuNextRow(addonsOptionsMenu);
		}

		OptionsMenuAddSection(addonsOptionsMenu, "Disabled Addons");
		if (disabledAddons.length == 0)
		{
			OptionsMenuAddControl(addonsOptionsMenu,
								  CreateLabelControl("(none)",
													 16,
													 COLOR(0xFF808080),
													 v2(0, 0),
													 v2(750, 16),
													 TOP_CENTER,
													 FONT_HALIGN_LEFT,
													 FONT_VALIGN_MIDDLE,
													 FONT("small_font"),
													 true));
		}
		for (size_t i = 0; i < disabledAddons.length; i++)
		{
			Addon *a = ListGetPointer(disabledAddons, i);
			const char *iconName = GetAddonIcon(a);
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
			Control *enableBtn = CreateButtonControl(v2(0, 28), v2(120, 40), "Enable", EnableAddonBtn, TOP_RIGHT, NULL);
			((ButtonData *)enableBtn->controlData)->extraData = a;
			OptionsMenuAddControl(addonsOptionsMenu, enableBtn);

			OptionsMenuNextRow(addonsOptionsMenu);
		}
		ListFree(disabledAddons);
	}

	OptionsMenuAddNoButtonHeaderFooter(addonsOptionsMenu, "Addons");
	UiStackPush(addonsOptionsMenu->stack,
				CreateButtonControl(v2(0, -40), v2(480, 40), "Done", DoneButton, BOTTOM_CENTER, NULL));
	UiStackResetFocus(addonsOptionsMenu->stack);
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
	if (shouldRebuildUiStack)
	{
		ReconstructUiStack();
	}

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
	ReconstructUiStack();
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
