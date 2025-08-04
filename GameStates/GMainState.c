//
// Created by droc101 on 4/22/2024.
//

#include "GMainState.h"
#include <math.h>
#include <stdio.h>
#include "../Debug/DPrint.h"
#include "../Debug/JoltDebugRenderer.h"
#include "../Helpers/Collision.h"
#include "../Helpers/CommonAssets.h"
#include "../Helpers/Core/AssetReader.h"
#include "../Helpers/Core/Error.h"
#include "../Helpers/Core/Input.h"
#include "../Helpers/Core/LodThread.h"
#include "../Helpers/Core/Logging.h"
#include "../Helpers/Core/MathEx.h"
#include "../Helpers/Core/PhysicsThread.h"
#include "../Helpers/Graphics/Drawing.h"
#include "../Helpers/Graphics/Font.h"
#include "../Helpers/Graphics/RenderingHelpers.h"
#include "../Structs/Actor.h"
#include "../Structs/GlobalState.h"
#include "../Structs/Level.h"
#include "../Structs/Vector2.h"
#include "GPauseState.h"

static Actor *targetedEnemy = NULL;
static bool lodThreadInitDone = false;

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void GMainStateUpdate(GlobalState *state)
{
	if (IsKeyJustPressed(SDL_SCANCODE_ESCAPE) || IsButtonJustPressed(SDL_CONTROLLER_BUTTON_START))
	{
		PlaySoundEffect(SOUND("sfx_popup"));
		GPauseStateSet();
		return;
	}

	state->level->player.transform.rotation.y -= GetMouseRel().x * (float)state->options.mouseSpeed / 120.0f;

	if (state->saveData->coins > 9999)
	{
		state->saveData->coins = 9999;
	}
	if (state->saveData->blueCoins > 5)
	{
		state->saveData->blueCoins = 5;
	}
}

static void MovePlayer(const Player *player, float *distanceTraveled)
{
	Vector2 moveVec = v2s(0);

	if (UseController())
	{
		moveVec.y = GetAxis(SDL_CONTROLLER_AXIS_LEFTX);
		moveVec.x = -GetAxis(SDL_CONTROLLER_AXIS_LEFTY);
		if (fabsf(moveVec.x) < STICK_DEADZONE)
		{
			moveVec.x = 0;
		}
		if (fabsf(moveVec.y) < STICK_DEADZONE)
		{
			moveVec.y = 0;
		}
	} else
	{
		if (IsKeyPressed(SDL_SCANCODE_W))
		{
			moveVec.y -= 1;
		} else if (IsKeyPressed(SDL_SCANCODE_S))
		{
			moveVec.y += 1;
		}

		if (IsKeyPressed(SDL_SCANCODE_D))
		{
			moveVec.x += 1;
		} else if (IsKeyPressed(SDL_SCANCODE_A))
		{
			moveVec.x -= 1;
		}
	}

	if (moveVec.x != 0 || moveVec.y != 0)
	{
		moveVec = Vector2Normalize(moveVec);
		*distanceTraveled = MOVE_SPEED;
		if (IsKeyPressed(SDL_SCANCODE_LCTRL) || GetAxis(SDL_CONTROLLER_AXIS_TRIGGERLEFT) > 0.5)
		{
			*distanceTraveled = SLOW_MOVE_SPEED;
		}
		moveVec = Vector2Rotate(Vector2Scale(moveVec, *distanceTraveled), -player->transform.rotation.y);
	}
	JPH_CharacterVirtual_SetLinearVelocity(player->joltCharacter, (Vector3[]){{moveVec.x, 0.0f, moveVec.y}});
}

void GMainStateFixedUpdate(GlobalState *state, const double delta)
{
	float distanceTraveled = 0;
	MovePlayer(&state->level->player, &distanceTraveled);

	// TODO: Why is controller rotation handed on the physics thread
	if (UseController())
	{
		float cx = GetAxis(SDL_CONTROLLER_AXIS_RIGHTX);
		if (state->options.cameraInvertX)
		{
			cx *= -1;
		}
		if (fabsf(cx) > STICK_DEADZONE)
		{
			state->level->player.transform.rotation.y += cx * (float)state->options.mouseSpeed / 11.25f;
		}
	}

	const float bobHeight = remap(distanceTraveled, 0, MOVE_SPEED / PHYSICS_TARGET_TPS, 0, 0.00175);
	state->camera->yOffset = 0.1f + (float)sin((double)state->physicsFrame / 7.0) * bobHeight;

	state->level->player.transform.rotation.y = wrap(state->level->player.transform.rotation.y, 0, 2 * PI);

	const float deltaTime = (float)delta / PHYSICS_TARGET_TPS;

	JPH_CharacterVirtual_Update(state->level->player.joltCharacter, deltaTime, OBJECT_LAYER_PLAYER, state->level->physicsSystem, JPH_BodyFilter_Create(NULL), JPH_ShapeFilter_Create(NULL));

	if (WaitForLodThreadToEnd() != 0)
	{
		Error("Failed to wait for LOD thread end semaphore!");
	}
	// WARNING: Any access to `state->level->actors` with ANY chance of modifying it MUST not happen before this!

	JPH_CharacterVirtual_GetPosition(state->level->player.joltCharacter, &state->level->player.transform.position);

	for (int i = 0; i < state->level->actors.length; i++)
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

	targetedEnemy = GetTargetedEnemy(10);
	if (targetedEnemy)
	{
		if (IsMouseButtonJustPressedPhys(SDL_BUTTON_LEFT) || IsButtonJustPressedPhys(SDL_CONTROLLER_BUTTON_X))
		{
			RemoveActor(targetedEnemy);
		}
	}

	// WARNING: Any access to `state->level->actors` with ANY chance of modifying it MUST not happen after this!
	if (SignalLodThreadCanStart() != 0)
	{
		Error("Failed to signal LOD thread start semaphore!");
	}

	// This is safe to be here because it does not modify the actors in any way.
	const JPH_PhysicsUpdateError result = JPH_PhysicsSystem_Update(state->level->physicsSystem,
																   deltaTime,
																   2,
																   state->jobSystem);
	if (result != JPH_PhysicsUpdateError_None)
	{
		LogError("Failed to update Jolt physics system with error %d\n", result);
		Error("Failed to update physics!");
	}
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void GMainStateRender(GlobalState *state)
{
	const Level *level = state->level;

	JoltDebugRendererDrawBodies(level->physicsSystem);
	RenderLevel3D(level, state->camera);

	SDL_Rect coinIconRect = {WindowWidth() - 260, 16, 40, 40};
	DrawTexture(v2(WindowWidthFloat() - 260, 16), v2(40, 40), TEXTURE("interface_hud_ycoin"));

	char coinStr[16];
	sprintf(coinStr, "%d", state->saveData->coins);
	FontDrawString(v2(WindowWidthFloat() - 210, 16), coinStr, 40, COLOR_WHITE, largeFont);

	coinIconRect.y = 64;

	for (int blueCoinIndex = 0; blueCoinIndex < state->saveData->blueCoins; blueCoinIndex++)
	{
		coinIconRect.x = WindowWidth() - 260 + blueCoinIndex * 48;
		DrawTexture(v2((float)coinIconRect.x, (float)coinIconRect.y), v2(40, 40), TEXTURE("interface_hud_bcoin"));
	}

	Color crosshairColor = COLOR(0xFFFFCCCC);
	if (targetedEnemy != NULL)
	{
		crosshairColor = COLOR(0xFFFF0000);
	}

	DrawTextureMod(v2((WindowWidth() * 0.5) - 12, (WindowHeight() * 0.5) - 12),
				   v2s(24),
				   TEXTURE("interface_crosshair"),
				   crosshairColor);

	DPrintF("Position: (%.2f, %.2f, %.2f)\nRotation: %.4f (%.2fdeg)",
			COLOR_WHITE,
			false,
			level->player.transform.position.x,
			level->player.transform.position.y,
			level->player.transform.position.z,
			fabsf(level->player.transform.rotation.y),
			radToDeg(fabsf(level->player.transform.rotation.y)));

	DPrintF("Walls: %d", COLOR_WHITE, false, level->walls.length);
	DPrintF("Actors: %d", COLOR_WHITE, false, level->actors.length);
	DPrintF("Targeted Actor: %p", COLOR_WHITE, false, targetedEnemy);
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
