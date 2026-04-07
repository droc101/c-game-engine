//
// Created by droc101 on 7/4/2024.
//

#include "gameState/LevelSelectState.h"
#include <engine/assets/AssetReader.h>
#include <engine/assets/GameConfigLoader.h>
#include <engine/graphics/Drawing.h>
#include <engine/graphics/Font.h>
#include <engine/graphics/RenderingHelpers.h>
#include <engine/helpers/MathEx.h>
#include <engine/physics/MapPhysics.h>
#include <engine/structs/Color.h>
#include <engine/structs/GameState.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/List.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Discord.h>
#include <engine/subsystem/Input.h>
#include <gameState/LoadingState.h>
#include <gameState/MenuState.h>
#include <SDL3/SDL_gamepad.h>
#include <SDL3/SDL_scancode.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

int selectedLevel = 0;
List levelList;

void LevelSelectStateUpdate(GlobalState *state)
{
	MapUpdate(state);

	if (IsKeyJustPressed(mainThreadInput, SDL_SCANCODE_ESCAPE) ||
		IsButtonJustPressed(mainThreadInput, CONTROLLER_CANCEL))
	{
		menuStateFadeIn = false;
		SetGameState(&MenuState);
	}
	if (levelList.length > 1)
	{
		if (IsKeyJustPressed(mainThreadInput, SDL_SCANCODE_DOWN) ||
			IsButtonJustPressed(mainThreadInput, SDL_GAMEPAD_BUTTON_DPAD_DOWN) ||
			GetMouseWheelTicks(mainThreadInput).y < 0)
		{
			selectedLevel++;
			selectedLevel = wrap(selectedLevel, 0, levelList.length);
		} else if (IsKeyJustPressed(mainThreadInput, SDL_SCANCODE_UP) ||
				   IsButtonJustPressed(mainThreadInput, SDL_GAMEPAD_BUTTON_DPAD_UP) ||
				   GetMouseWheelTicks(mainThreadInput).y > 0)
		{
			selectedLevel--;
			selectedLevel = wrap(selectedLevel, 0, levelList.length);
		}
	}
	if (levelList.length != 0 && (IsKeyJustReleased(mainThreadInput, SDL_SCANCODE_SPACE) ||
								  IsButtonJustReleased(mainThreadInput, CONTROLLER_OK)))
	{
		ConsumeKey(mainThreadInput, SDL_SCANCODE_SPACE);
		ConsumeButton(mainThreadInput, CONTROLLER_OK);
		loadStateLevelname = strdup(ListGetPointer(levelList, selectedLevel));
		SetGameState(&LoadingState);
	}
}

void LevelSelectStateRender(GlobalState * /*state*/)
{
	RenderMenuBackground();

	FontDrawString(v2(52, 52), "Map Select", 64, COLOR_BLACK, smallFont);
	FontDrawString(v2(50, 50), "Map Select", 64, COLOR_WHITE, smallFont);

	DrawRect(0, 315, ScaledWindowWidth(), 120, COLOR(0x80000000));
	if (levelList.length == 0)
	{
		DrawTextAligned("No Levels Found",
						32,
						COLOR_WHITE,
						v2(50, 345),
						v2(ScaledWindowWidthFloat() - 50, 60),
						FONT_HALIGN_LEFT,
						FONT_VALIGN_MIDDLE,
						smallFont);
	} else
	{
		for (size_t i = 0; i < levelList.length; i++)
		{
			const float yPos = (float)(345 + ((i - selectedLevel) * 60));
			DrawTextAligned(ListGetPointer(levelList, i),
							32,
							(int)i == selectedLevel ? COLOR_WHITE : COLOR(0x80ffffff),
							v2(50, yPos),
							v2(ScaledWindowWidthFloat() - 50, 60),
							FONT_HALIGN_LEFT,
							FONT_VALIGN_MIDDLE,
							smallFont);
		}
		char progress[64];
		snprintf(progress, 64, "Map %02d/%02zu", selectedLevel + 1, levelList.length);
		FontDrawString(v2(50, 325), progress, 16, COLOR_WHITE, smallFont);
		FontDrawString(v2(50, 409), "Up/Down to change, space to play", 16, COLOR_WHITE, smallFont);
	}
}

void LoadLevelList()
{
	ListInit(levelList, LIST_POINTER);
	EnumerateAssetsInFolder("map", &levelList, ".gmap");
}

void LevelSelectStateSet()
{
	GetState()->rpcState = IN_MENUS;
	if (levelList.length == 0)
	{
		LoadLevelList();
	}
	if (!GetState()->map || strcmp(GetState()->map->mapName, "background") != 0)
	{
		ChangeMapByName("background");
	}
}

void LevelSelectStateDestroy()
{
	ListAndContentsFree(levelList);
}

const GameState LevelSelectState = {.UpdateGame = LevelSelectStateUpdate,
									.RenderGame = LevelSelectStateRender,
									.FixedUpdateGame = MapFixedUpdate,
									.Set = LevelSelectStateSet,
									.Destroy = LevelSelectStateDestroy,
									.enableRelativeMouseMode = false};
