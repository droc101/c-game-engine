//
// Created by droc101 on 7/19/26.
//

#include <engine/assets/KvlFile.h>
#include <engine/debug/DebugEntryManager.h>
#include <engine/debug/DebugGraph.h>
#include <engine/debug/DPrint.h>
#include <engine/debug/DPrintConsole.h>
#include <engine/debug/FrameGrapher.h>
#include <engine/Engine.h>
#include <engine/graphics/RenderingHelpers.h>
#include <engine/structs/Camera.h>
#include <engine/structs/Color.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/KVList.h>
#include <engine/structs/List.h>
#include <engine/structs/Player.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Input.h>
#include <engine/subsystem/SoundSystem.h>
#include <engine/subsystem/threads/PhysicsThread.h>
#include <joltc/joltc.h>
#include <joltc/Math/Quat.h>
#include <joltc/Math/Vector3.h>
#include <SDL3/SDL_cpuinfo.h>
#include <SDL3/SDL_platform.h>
#include <SDL3/SDL_stdinc.h>
#include <stdbool.h>
#include <stdlib.h>

#include "engine/subsystem/Logging.h"

List debugEntries;

static bool expandedMenu = false;

#define DEBUG_OPTIONS_FILE "debug_options.kvl"

static void RegisterDebugEntry(const char *key,
							   const DebugEntryFunction entry,
							   const DebugEntryMode defaultMode,
							   const int spacing)
{
	DebugEntry *e = malloc(sizeof(DebugEntry));
	CheckAlloc(e);
	e->key = strdup(key);
	CheckAlloc(e->key);
	e->process = entry;
	e->mode = defaultMode;
	e->defaultMode = defaultMode;
	e->spacing = spacing;
	ListAdd(debugEntries, e);
}

#pragma region Debug Entries

static void DebugEntryVersion()
{
	DPrintF("Engine " ENGINE_VERSION, COLOR_WHITE);
#ifdef BUILDSTYLE_DEBUG
	DPrintF("DEBUG BUILD", COLOR(0xFF00FF00));
#endif
}

static void DebugEntryGitCommit()
{
	DPrintF("Commit: " ENGINE_GIT_HASH, COLOR_WHITE);
}

static void DebugEntryMap()
{
	if (GetState()->map)
	{
		DPrintF("Map: %s", COLOR_WHITE, GetState()->map->mapName);
		DPrintF("Actors: %d", COLOR_WHITE, GetState()->map->actors.length);
		DPrintF("Models: %d", COLOR_WHITE, GetState()->map->modelCount);
		DPrintF("Lights: %d", COLOR_WHITE, GetState()->map->lightCount);
	}
}

static void DebugEntryCamera()
{
	if (GetState()->camera)
	{
		const Camera *cam = GetState()->camera;
		DPrintF("Camera", COLOR_WHITE);
		DPrintF("Position: %.2f %.2f %.2f",
				COLOR_WHITE,
				cam->transform.position.x,
				cam->transform.position.y,
				cam->transform.position.z);
		Vector3 camRot;
		JPH_Quat_GetEulerAngles(&cam->transform.rotation, &camRot);
		DPrintF("Rotation: %.2f %.2f %.2f", COLOR_WHITE, glm_deg(camRot.x), glm_deg(camRot.y), glm_deg(camRot.z));
		DPrintF("FOV: %.2f", COLOR_WHITE, cam->fov);
	}
}

static void DebugEntryPlayerPosition()
{
	if (GetState()->map)
	{
		Player *player = &GetState()->map->player;
		DPrintF("Position: %.2f %.2f %.2f",
				COLOR_WHITE,
				player->transform.position.x,
				player->transform.position.y,
				player->transform.position.z);
	}
}

static void DebugEntryPlayerVelocity()
{
	if (GetState()->map)
	{
		Player *player = &GetState()->map->player;

		Vector3 playerVelocity;
		JPH_CharacterVirtual_GetLinearVelocity(player->joltCharacter, &playerVelocity);
		const float totalVelocity = Vector3_Length(&playerVelocity);
		DPrintF("Velocity: %.2f (%.2f, %.2f, %.2f)",
				COLOR_WHITE,
				totalVelocity,
				playerVelocity.x,
				playerVelocity.y,
				playerVelocity.z);
	}
}

static void DebugEntryPlayerActor()
{
	if (GetState()->map)
	{
		Player *player = &GetState()->map->player;

		PhysicsThreadLockTickMutex();
		DPrintF("%s Actor: %s %p",
				COLOR_WHITE,
				player->hasHeldActor ? "Held" : "Targeted",
				player->targetedActor ? player->targetedActor->definition->className : "None",
				player->targetedActor);
		PhysicsThreadUnlockTickMutex();
	}
}

static void DebugEntryFPS()
{
	DPrintF("%.0f FPS (%.2fms) / max %d%s",
			COLOR_WHITE,
			DebugGraphGetValue(fpsGraph),
			DebugGraphGetLinearValue(fpsGraph),
			GetState()->options.maxFps,
			GetState()->options.vsync ? " vsync" : "");
}

static void DebugEntryTPS()
{
	if (GetState()->gameState->FixedUpdateGame)
	{
		DPrintF("%.0f TPS (%.2fms)", COLOR_WHITE, DebugGraphGetValue(tpsGraph), DebugGraphGetLinearValue(tpsGraph));
	}
}

static void DebugEntrySystem()
{
	DPrintF("Platform: %s", COLOR_WHITE, SDL_GetPlatform());
	DPrintF("CPUs: %d", COLOR_WHITE, SDL_GetNumLogicalCPUCores());
	DPrintF("Memory: %d MiB", COLOR_WHITE, SDL_GetSystemRAM());
	const Vector2 viewport = ActualWindowSizeIgnoreDPI();
	DPrintF("Viewport: %.0fx%.0f", COLOR_WHITE, viewport.x, viewport.y);
	DPrintGPUInfo();
}

#pragma endregion

void InitDebugEntryManager()
{
	ListInit(debugEntries, LIST_POINTER);
	RegisterDebugEntry("engine_version", DebugEntryVersion, DEBUG_ENTRY_SHOWN, 5);
	RegisterDebugEntry("git_commit", DebugEntryGitCommit, DEBUG_ENTRY_DISABLED, 5);
	RegisterDebugEntry("fps", DebugEntryFPS, DEBUG_ENTRY_SHOWN, 5);
	RegisterDebugEntry("tps", DebugEntryTPS, DEBUG_ENTRY_SHOWN, 5);
	RegisterDebugEntry("fps_graph", FrameGraphDraw, DEBUG_ENTRY_TOGGLE, 0);
	RegisterDebugEntry("tps_graph", TickGraphDraw, DEBUG_ENTRY_TOGGLE, 0);
	RegisterDebugEntry("player_position", DebugEntryPlayerPosition, DEBUG_ENTRY_TOGGLE, 5);
	RegisterDebugEntry("player_velocity", DebugEntryPlayerVelocity, DEBUG_ENTRY_TOGGLE, 5);
	RegisterDebugEntry("player_actor_interaction", DebugEntryPlayerActor, DEBUG_ENTRY_TOGGLE, 5);
	RegisterDebugEntry("map", DebugEntryMap, DEBUG_ENTRY_DISABLED, 5);
	RegisterDebugEntry("camera", DebugEntryCamera, DEBUG_ENTRY_DISABLED, 5);
	RegisterDebugEntry("system_specs", DebugEntrySystem, DEBUG_ENTRY_DISABLED, 5);
	RegisterDebugEntry("sound_system", DPrintSoundSystem, DEBUG_ENTRY_DISABLED, 5);
	RegisterDebugEntry("console", DrawDPrintConsole, DEBUG_ENTRY_SHOWN, 5);

	KvList list;
	if (ReadKvlFile(DEBUG_OPTIONS_FILE, list))
	{
		KvList debugEntrySettings;
		if (KvGetList(list, "debug_entries", debugEntrySettings))
		{
			for (size_t i = 0; i < debugEntries.length; i++)
			{
				DebugEntry *ent = ListGetPointer(debugEntries, i);
				ent->mode = KvGetByte(debugEntrySettings, ent->key, ent->defaultMode);
				if (ent->mode >= DEBUG_ENTRY_MODE_MAX)
				{
					LogWarning("Invalid saved mode %d for debug entry \"%s\"\n", ent->mode, ent->key);
					ent->mode = ent->defaultMode;
				}
			}
		}
		expandedMenu = KvGetBool(list, "extended_menu_visible", false);

		KvListDestroy(debugEntrySettings);
		KvListDestroy(list);
	}
}

void RenderDebugEntries()
{
	if (IsKeyJustPressed(mainThreadInput, SDL_SCANCODE_F4))
	{
		expandedMenu = !expandedMenu;
	}

	for (size_t i = 0; i < debugEntries.length; i++)
	{
		const DebugEntry *ent = ListGetPointer(debugEntries, i);
		if (ent->mode == DEBUG_ENTRY_SHOWN || (ent->mode == DEBUG_ENTRY_TOGGLE && expandedMenu))
		{
			ent->process();
			DPrintSpacing(ent->spacing);
		}
	}
}

void SaveDebugEntrySettings()
{
	KvList list;
	KvListCreate(list);

	KvList debugEntrySettings;
	KvListCreate(debugEntrySettings);
	for (size_t i = 0; i < debugEntries.length; i++)
	{
		const DebugEntry *ent = ListGetPointer(debugEntries, i);
		KvSetByte(debugEntrySettings, ent->key, ent->mode);
	}
	KvSetList(list, "debug_entries", debugEntrySettings);
	KvSetBool(list, "extended_menu_visible", expandedMenu);

	(void)WriteKvlFile(DEBUG_OPTIONS_FILE, list);
	KvListDestroy(debugEntrySettings);
}

void DestroyDebugEntryManager()
{
	SaveDebugEntrySettings();

	for (size_t i = 0; i < debugEntries.length; i++)
	{
		const DebugEntry *ent = ListGetPointer(debugEntries, i);
		free(ent->key);
	}

	ListAndContentsFree(debugEntries);
}
