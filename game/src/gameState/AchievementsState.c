//
// Created by droc101 on 8/27/26.
//

#include "gameState/AchievementsState.h"
#include <assert.h>
#include <engine/assets/AssetReader.h>
#include <engine/graphics/Font.h>
#include <engine/helpers/BackgroundMapManager.h>
#include <engine/structs/Achievements.h>
#include <engine/structs/Color.h>
#include <engine/structs/GameState.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/List.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Input.h>
#include <engine/uiStack/controls/Image.h>
#include <engine/uiStack/controls/LabelControl.h>
#include <engine/uiStack/UiStack.h>
#include <SDL3/SDL_scancode.h>
#include <stdbool.h>
#include <stddef.h>
#include <time.h>
#include "gameState/MenuState.h"
#include "helpers/OptionsMenu.h"

static OptionsMenu *achievementsOptionsMenu = NULL;

static void DoneButton(Control *, void *)
{
	SetGameState(&MenuState);
}

static void AchievementsStateUpdate(GlobalState *state, const double delta)
{
	if (IsKeyJustPressed(mainThreadInput, SDL_SCANCODE_ESCAPE) ||
		IsButtonJustPressed(mainThreadInput, CONTROLLER_CANCEL))
	{
		DoneButton(NULL, NULL);
	}
	UpdateMenuBackground(state, delta);
}

static void AchievementsStateRender(GlobalState *state, const double /*delta*/)
{
	RenderMenuBackground(state, true);
	if (!IsBackgroundMapLoaded())
	{
		return;
	}

	ProcessOptionsMenu(achievementsOptionsMenu);
}

static void AchievementsStateSet()
{
	if (achievementsOptionsMenu == NULL)
	{
		achievementsOptionsMenu = CreateOptionsMenu();

		List allAchievements;
		List unlockedAchievements;
		List lockedAchievements;
		size_t hiddenAchievements = 0;
		ListInit(allAchievements, LIST_POINTER);
		ListInit(unlockedAchievements, LIST_POINTER);
		ListInit(lockedAchievements, LIST_POINTER);
		GetAchievementKeys(&allAchievements);
		for (size_t i = 0; i < allAchievements.length; i++)
		{
			const char *key = ListGetPointer(allAchievements, i);
			const Achievement *ach = GetAchievement(key);
			if (IsAchievementUnlocked(key))
			{
				ListAdd(unlockedAchievements, key);
			} else
			{
				if (ach->hideUntilUnlocked)
				{
					hiddenAchievements++;
				} else
				{
					ListAdd(lockedAchievements, key);
				}
			}
		}
		ListFree(allAchievements);

		OptionsMenuAddSection(achievementsOptionsMenu, "Unlocked Achievements");
		if (unlockedAchievements.length == 0)
		{
			OptionsMenuAddControl(achievementsOptionsMenu,
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
			OptionsMenuNextRow(achievementsOptionsMenu);
		} else
		{
			for (size_t i = 0; i < unlockedAchievements.length; i++)
			{
				const char *key = ListGetPointer(unlockedAchievements, i);
				const char *iconName = GetAchievementIcon(key);
				const Achievement *ach = GetAchievement(key);
				Control *icon = CreateImageControl(v2(0, 0), v2s(64), iconName, TOP_LEFT, NULL);
				OptionsMenuAddControl(achievementsOptionsMenu, icon);
				OptionsMenuAddControl(achievementsOptionsMenu,
									  CreateLabelControl(ach->name,
														 16,
														 COLOR_WHITE,
														 v2(72, 0),
														 v2(750 - 72, 32),
														 TOP_LEFT,
														 FONT_HALIGN_LEFT,
														 FONT_VALIGN_MIDDLE,
														 FONT("small_font"),
														 true));
				OptionsMenuAddControl(achievementsOptionsMenu,
									  CreateLabelControl(ach->description,
														 16,
														 COLOR(0xFF808080),
														 v2(72, 40),
														 v2(750 - 72, 64 - 40),
														 TOP_LEFT,
														 FONT_HALIGN_LEFT,
														 FONT_VALIGN_TOP,
														 FONT("small_font"),
														 true));

				OptionsMenuNextRow(achievementsOptionsMenu);
			}
		}

		ListFree(unlockedAchievements);

		OptionsMenuAddSection(achievementsOptionsMenu, "Locked Achievements");
		if (lockedAchievements.length == 0)
		{
			if (hiddenAchievements == 0)
			{
				OptionsMenuAddControl(achievementsOptionsMenu,
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
			} else
			{
				OptionsMenuAddControl(achievementsOptionsMenu,
									  CreateLabelControl("Some hidden achievements...",
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
			OptionsMenuNextRow(achievementsOptionsMenu);
		} else
		{
			for (size_t i = 0; i < lockedAchievements.length; i++)
			{
				const char *key = ListGetPointer(lockedAchievements, i);
				const char *iconName = GetAchievementIcon(key);
				const Achievement *ach = GetAchievement(key);
				Control *icon = CreateImageControl(v2(0, 0), v2s(64), iconName, TOP_LEFT, NULL);
				OptionsMenuAddControl(achievementsOptionsMenu, icon);
				OptionsMenuAddControl(achievementsOptionsMenu,
									  CreateLabelControl(ach->name,
														 16,
														 COLOR_WHITE,
														 v2(72, 0),
														 v2(750 - 72, 32),
														 TOP_LEFT,
														 FONT_HALIGN_LEFT,
														 FONT_VALIGN_MIDDLE,
														 FONT("small_font"),
														 true));
				OptionsMenuAddControl(achievementsOptionsMenu,
									  CreateLabelControl(ach->description,
														 16,
														 COLOR(0xFF808080),
														 v2(72, 40),
														 v2(750 - 72, 64 - 40),
														 TOP_LEFT,
														 FONT_HALIGN_LEFT,
														 FONT_VALIGN_TOP,
														 FONT("small_font"),
														 true));

				OptionsMenuNextRow(achievementsOptionsMenu);
			}
			if (hiddenAchievements != 0)
			{
				OptionsMenuAddControl(achievementsOptionsMenu,
									  CreateLabelControl("...and some hidden achievements",
														 16,
														 COLOR(0xFF808080),
														 v2(0, 0),
														 v2(750, 16),
														 TOP_CENTER,
														 FONT_HALIGN_LEFT,
														 FONT_VALIGN_MIDDLE,
														 FONT("small_font"),
														 true));
				OptionsMenuNextRow(achievementsOptionsMenu);
			}
		}
		ListFree(lockedAchievements);


		OptionsMenuAddSimpleHeaderFooter(achievementsOptionsMenu, "Achievements", DoneButton);
	}
	EnterMenuBackgroundState();
}

static void AchievementsStateDestroy()
{
	if (achievementsOptionsMenu != NULL)
	{
		DestroyOptionsMenu(achievementsOptionsMenu);
		achievementsOptionsMenu = NULL;
	}
}

const GameState AchievementsState = {
	.UpdateGame = AchievementsStateUpdate,
	.RenderGame = AchievementsStateRender,
	.FixedUpdateGame = FixedUpdateMenuBackground,
	.Destroy = AchievementsStateDestroy,
	.Set = AchievementsStateSet,
	.enableRelativeMouseMode = false,
};
