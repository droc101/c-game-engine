//
// Created by droc101 on 7/4/2024.
//

#include "gameState/LevelSelectState.h"
#include <dirent.h>
#include <engine/assets/GameConfigLoader.h>
#include <engine/graphics/Drawing.h>
#include <engine/graphics/Font.h>
#include <engine/graphics/RenderingHelpers.h>
#include <engine/helpers/MathEx.h>
#include <engine/structs/Color.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/List.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Discord.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Input.h>
#include <engine/subsystem/Logging.h>
#include <engine/subsystem/SoundSystem.h>
#include <SDL_gamecontroller.h>
#include <SDL_scancode.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gameState/LoadingState.h"
#include "gameState/MenuState.h"

int selectedLevel = 0;
List levelList;

void LevelSelectStateUpdate(GlobalState * /*state*/)
{
	if (IsKeyJustPressed(SDL_SCANCODE_ESCAPE) || IsButtonJustPressed(CONTROLLER_CANCEL))
	{
		MenuStateSet();
	}
	if (levelList.length > 1)
	{
		if (IsKeyJustPressed(SDL_SCANCODE_DOWN) || IsButtonJustPressed(SDL_CONTROLLER_BUTTON_DPAD_DOWN))
		{
			selectedLevel--;
			selectedLevel = wrap(selectedLevel, 0, levelList.length);
		} else if (IsKeyJustPressed(SDL_SCANCODE_UP) || IsButtonJustPressed(SDL_CONTROLLER_BUTTON_DPAD_UP))
		{
			selectedLevel++;
			selectedLevel = wrap(selectedLevel, 0, levelList.length);
		}
	}
	if (levelList.length != 0 && (IsKeyJustReleased(SDL_SCANCODE_SPACE) || IsButtonJustReleased(CONTROLLER_OK)))
	{
		ConsumeKey(SDL_SCANCODE_SPACE);
		ConsumeButton(CONTROLLER_OK);
		LoadingStateSet(ListGetPointer(levelList, selectedLevel));
	}
}

void LevelSelectStateRender(GlobalState * /*state*/)
{
	RenderMenuBackground();

	FontDrawString(v2(20, 20), config.gameTitle, 128, COLOR_WHITE, largeFont);
	FontDrawString(v2(20, 150), "Press Space to start.", 32, COLOR(0xFFa0a0a0), largeFont);

	char levelNameBuffer[128];

	if (levelList.length > 0)
	{
		char *levelName = ListGetPointer(levelList, selectedLevel);

		snprintf(levelNameBuffer, 128, "%s", levelName);
	} else
	{
		strcpy((char *)&levelNameBuffer, "No levels found");
	}

	DrawTextAligned(levelNameBuffer,
					32,
					COLOR_WHITE,
					v2(50, 250),
					v2(ScaledWindowWidthFloat() - 50, 250),
					FONT_HALIGN_LEFT,
					FONT_VALIGN_MIDDLE,
					smallFont);
}

void LoadLevelList()
{
	ListInit(levelList, LIST_POINTER);
	char levelDataPath[300];
	sprintf(levelDataPath, "%sassets/level/", GetState()->executableFolder);

	// Get the name of all gmap files in the level directory
	DIR *dir = opendir(levelDataPath);
	if (dir == NULL)
	{
		LogError("Failed to open level directory: %s\n", levelDataPath);
		return;
	}

	const struct dirent *ent = readdir(dir);
	while (ent != NULL)
	{
		if (strstr(ent->d_name, ".gmap") != NULL)
		{
			char *levelName = malloc(strlen(ent->d_name) + 1);
			CheckAlloc(levelName);
			strcpy(levelName, ent->d_name);
			// Remove the .gmap extension
			levelName[strlen(levelName) - 5] = '\0';
			ListAdd(levelList, levelName);
		}
		ent = readdir(dir);
	}
	closedir(dir);
}

void LevelSelectStateSet()
{
	GetState()->rpcState = IN_MENUS;
	if (levelList.length == 0)
	{
		LoadLevelList();
	}
	StopMusic();
	SetStateCallbacks(LevelSelectStateUpdate,
					  NULL,
					  GAME_STATE_LEVEL_SELECT,
					  LevelSelectStateRender,
					  SDL_FALSE); // Fixed update is not needed for this state
}

void LevelSelectStateDestroy()
{
	ListFreeOnlyContents(levelList);
}
