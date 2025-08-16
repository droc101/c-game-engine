//
// Created by droc101 on 4/22/2024.
//

#include "GMainState.h"
#include <joltc/enums.h>
#include <joltc/joltc.h>
#include <math.h>
#include <SDL_gamecontroller.h>
#include <SDL_rect.h>
#include <SDL_scancode.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include "../config.h"
#include "../Debug/DPrint.h"
#include "../Debug/JoltDebugRenderer.h"
#include "../Helpers/CommonAssets.h"
#include "../Helpers/Core/AssetReader.h"
#include "../Helpers/Core/Error.h"
#include "../Helpers/Core/Input.h"
#include "../Helpers/Core/List.h"
#include "../Helpers/Core/Logging.h"
#include "../Helpers/Core/MathEx.h"
#include "../Helpers/Core/Physics/Player.h"
#include "../Helpers/Core/SoundSystem.h"
#include "../Helpers/Graphics/Drawing.h"
#include "../Helpers/Graphics/Font.h"
#include "../Helpers/Graphics/LodThread.h"
#include "../Helpers/Graphics/RenderingHelpers.h"
#include "../Structs/Actor.h"
#include "../Structs/Color.h"
#include "../Structs/GlobalState.h"
#include "../Structs/Level.h"
#include "../Structs/Vector2.h"
#include "GPauseState.h"

static bool lodThreadInitDone = false;

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void GMainStateUpdate(GlobalState *state)
{
	if (IsKeyJustPressed(SDL_SCANCODE_ESCAPE) || IsButtonJustPressed(SDL_CONTROLLER_BUTTON_START))
	{
		(void)PlaySoundEffect(SOUND("sfx/popup"), 0, 1, NULL, NULL);
		GPauseStateSet();
		return;
	}

	Vector2 cameraMotion = GetMouseRel();
	cameraMotion.x *= state->options.cameraSpeed / 120.0f;
	cameraMotion.y *= state->options.cameraSpeed / 120.0f;
	if (state->options.invertHorizontalCamera)
	{
		cameraMotion.x *= -1;
	}
	if (state->options.invertVerticalCamera)
	{
		cameraMotion.y *= -1;
	}

	state->level->player.transform.rotation.y -= cameraMotion.x;
	state->level->player.transform.rotation.x -= cameraMotion.y;

	state->level->player.transform.rotation.y = wrap(state->level->player.transform.rotation.y, 0, 2 * PI);
	state->level->player.transform.rotation.x = clamp(state->level->player.transform.rotation.x, -PI / 2, PI / 2);

	if (state->saveData->coins > 9999)
	{
		state->saveData->coins = 9999;
	}
	if (state->saveData->blueCoins > 5)
	{
		state->saveData->blueCoins = 5;
	}
}

void GMainStateFixedUpdate(GlobalState *state, const double delta)
{
	float distanceTraveled = 0;
	MovePlayer(&state->level->player, &distanceTraveled);

	// TODO: Why is controller rotation handed on the physics thread
	if (UseController())
	{
		float cx = -GetAxis(SDL_CONTROLLER_AXIS_RIGHTX);
		if (state->options.invertHorizontalCamera)
		{
			cx *= -1;
		}
		if (fabsf(cx) > STICK_DEADZONE)
		{
			state->level->player.transform.rotation.y += cx * state->options.cameraSpeed / 11.25f;
		}

		float cy = -GetAxis(SDL_CONTROLLER_AXIS_RIGHTY);
		if (state->options.invertVerticalCamera)
		{
			cy *= -1;
		}
		if (fabsf(cy) > STICK_DEADZONE)
		{
			state->level->player.transform.rotation.x += cy * state->options.cameraSpeed / 11.25f;
		}

		state->level->player.transform.rotation.y = wrap(state->level->player.transform.rotation.y, 0, 2 * PI);
		state->level->player.transform.rotation.x = clamp(state->level->player.transform.rotation.x, -PI / 2, PI / 2);
	}

	const float bobHeight = remap(distanceTraveled, 0, MOVE_SPEED / PHYSICS_TARGET_TPS, 0, 0.00175);
	state->camera->yOffset = 0.1f + (float)sin((double)state->physicsFrame / 7.0) * bobHeight;

	if (WaitForLodThreadToEnd() != 0)
	{
		Error("Failed to wait for LOD thread end semaphore!");
	}
	// WARNING: Any access to `state->level->actors` with ANY chance of modifying it MUST not happen before this!

	const float deltaTime = (float)delta / PHYSICS_TARGET_TPS;

	UpdatePlayer(&state->level->player, state->level->physicsSystem, deltaTime);

	JPH_CharacterVirtual_GetPosition(state->level->player.joltCharacter, &state->level->player.transform.position);

	for (size_t i = 0; i < state->level->actors.length; i++)
	{
		Actor *a = ListGetPointer(state->level->actors, i);
		a->Update(a, delta);
	}

	if (IsKeyJustPressedPhys(SDL_SCANCODE_L))
	{
		Actor *leaf = CreateActor(&state->level->player.transform,
								  ACTOR_TYPE_TEST,
								  NULL,
								  JPH_PhysicsSystem_GetBodyInterface(state->level->physicsSystem));
		AddActor(leaf);
	}

	const JPH_PhysicsUpdateError result = JPH_PhysicsSystem_Update(state->level->physicsSystem,
																   deltaTime,
																   2,
																   state->jobSystem);
	if (result != JPH_PhysicsUpdateError_None)
	{
		LogError("Failed to update Jolt physics system with error %d\n", result);
		Error("Failed to update physics!");
	}

	// WARNING: Any access to `state->level->actors` with ANY chance of modifying it MUST not happen after this!
	if (SignalLodThreadCanStart() != 0)
	{
		Error("Failed to signal LOD thread start semaphore!");
	}
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void GMainStateRender(GlobalState *state)
{
	const Level *level = state->level;

	JoltDebugRendererDrawBodies(level->physicsSystem);
	RenderLevel3D(level, state->camera);

	SDL_Rect coinIconRect = {WindowWidth() - 260, 16, 40, 40};
	DrawTexture(v2(WindowWidthFloat() - 260, 16), v2(40, 40), TEXTURE("interface/hud_ycoin"));

	char coinStr[16];
	sprintf(coinStr, "%d", state->saveData->coins);
	FontDrawString(v2(WindowWidthFloat() - 210, 16), coinStr, 40, COLOR_WHITE, largeFont);

	coinIconRect.y = 64;

	for (int blueCoinIndex = 0; blueCoinIndex < state->saveData->blueCoins; blueCoinIndex++)
	{
		coinIconRect.x = WindowWidth() - 260 + blueCoinIndex * 48;
		DrawTexture(v2((float)coinIconRect.x, (float)coinIconRect.y), v2(40, 40), TEXTURE("interface/hud_bcoin"));
	}

	const Color *crosshairColor = GetCrosshairColor();

	DrawTextureMod(v2((WindowWidth() * 0.5) - 12, (WindowHeight() * 0.5) - 12),
				   v2s(24),
				   TEXTURE("interface/crosshair"),
				   crosshairColor);

	DPrintPlayer(level);

	DPrintF("Walls: %d", COLOR_WHITE, false, level->walls.length);
	DPrintF("Actors: %d", COLOR_WHITE, false, level->actors.length);
}

void GMainStateSet()
{
	if (!lodThreadInitDone)
	{
		LodThreadInit();
		lodThreadInitDone = true;
	}
	SetStateCallbacks(GMainStateUpdate, GMainStateFixedUpdate, MAIN_STATE, GMainStateRender);
}
