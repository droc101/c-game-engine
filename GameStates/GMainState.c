//
// Created by droc101 on 4/22/2024.
//

#include "GMainState.h"
#include <box2d/box2d.h>
#include <math.h>
#include <stdio.h>
#include "../Debug/DPrint.h"
#include "../Helpers/Collision.h"
#include "../Helpers/CommandParser.h"
#include "../Helpers/CommonAssets.h"
#include "../Helpers/Core/AssetReader.h"
#include "../Helpers/Core/Error.h"
#include "../Helpers/Core/Input.h"
#include "../Helpers/Core/MathEx.h"
#include "../Helpers/Graphics/Drawing.h"
#include "../Helpers/Graphics/Font.h"
#include "../Helpers/Graphics/RenderingHelpers.h"
#include "../Helpers/TextBox.h"
#include "../Structs/Actor.h"
#include "../Structs/GlobalState.h"
#include "../Structs/Level.h"
#include "../Structs/Vector2.h"
#include "GPauseState.h"

Actor *targetedEnemy = NULL;

void GMainStateUpdate(GlobalState *State)
{
	if (IsKeyJustPressed(SDL_SCANCODE_ESCAPE) || IsButtonJustPressed(SDL_CONTROLLER_BUTTON_START))
	{
		PlaySoundEffect(SOUND("sfx_popup"));
		GPauseStateSet();
		return;
	}

	if (State->textBoxActive)
	{
		if (IsKeyJustPressed(SDL_SCANCODE_SPACE) || IsButtonJustPressed(CONTROLLER_OK))
		{
			State->textBoxPage++;
			if (State->textBoxPage >= StringLineCount(State->textBox.text) / State->textBox.rows)
			{
				State->textBoxActive = false;
			}
		}
		return;
	}

	State->level->player.angle += GetMouseRel().x * (float)State->options.mouseSpeed / 120.0f;

	if (State->saveData->coins > 9999)
	{
		State->saveData->coins = 9999;
	}
	if (State->saveData->blueCoins > 5)
	{
		State->saveData->blueCoins = 5;
	}
}

void CalculateMoveVec(const double delta, const Player *player, Vector2 *moveVec, bool *isMoving)
{
	*moveVec = v2s(0);
	*isMoving = false;

	if (UseController())
	{
		moveVec->y = GetAxis(SDL_CONTROLLER_AXIS_LEFTX);
		moveVec->x = -GetAxis(SDL_CONTROLLER_AXIS_LEFTY);
		if (fabsf(moveVec->x) < STICK_DEADZONE)
		{
			moveVec->x = 0;
		}
		if (fabsf(moveVec->y) < STICK_DEADZONE)
		{
			moveVec->y = 0;
		}

	} else
	{
		if (IsKeyPressed(SDL_SCANCODE_W) || GetAxis(SDL_CONTROLLER_AXIS_LEFTY) < -0.5)
		{
			moveVec->x += 1;
		} else if (IsKeyPressed(SDL_SCANCODE_S) || GetAxis(SDL_CONTROLLER_AXIS_LEFTY) > 0.5)
		{
			moveVec->x -= 1;
		}

		if (IsKeyPressed(SDL_SCANCODE_A) || GetAxis(SDL_CONTROLLER_AXIS_LEFTX) < -0.5)
		{
			moveVec->y -= 1;
		} else if (IsKeyPressed(SDL_SCANCODE_D) || GetAxis(SDL_CONTROLLER_AXIS_LEFTX) > 0.5)
		{
			moveVec->y += 1;
		}
	}


	*isMoving = moveVec->x != 0 || moveVec->y != 0;

	if (*isMoving && !UseController())
	{
		*moveVec = Vector2Normalize(*moveVec);
	}


	float speed = MOVE_SPEED;
	if (IsKeyPressed(SDL_SCANCODE_LCTRL) || GetAxis(SDL_CONTROLLER_AXIS_TRIGGERLEFT) > 0.5)
	{
		speed = SLOW_MOVE_SPEED;
	}

	speed *= (float)delta;

	Vector2 rotScaled = Vector2Scale(*moveVec, speed);
	rotScaled = Vector2Rotate(rotScaled, player->angle);
	*moveVec = rotScaled;
}

void GMainStateFixedUpdate(GlobalState *state, const double delta)
{
	if (state->textBoxActive)
	{
		return;
	}

	Level *l = state->level;

	Vector2 moveVec;
	bool isMoving;
	CalculateMoveVec(delta, &l->player, &moveVec, &isMoving);

	if (isMoving)
	{
		b2Body_ApplyLinearImpulseToCenter(l->player.bodyId, moveVec, true);
	}

	if (UseController())
	{
		float cx = GetAxis(SDL_CONTROLLER_AXIS_RIGHTX);
		if (state->options.cameraInvertX)
		{
			cx *= -1;
		}
		if (fabsf(cx) > STICK_DEADZONE)
		{
			l->player.angle += cx * (float)state->options.mouseSpeed / 11.25f;
		}
	}

	const float velocity = Vector2Length(b2Body_GetLinearVelocity(l->player.bodyId));
	const float bobHeight = remap(velocity, 0, MOVE_SPEED, 0, 0.003);
	state->cameraY = 0.1 + sin((double)state->physicsFrame / 7.0) * bobHeight;

	l->player.angle = wrap(l->player.angle, 0, 2 * PI);

	for (int i = 0; i < l->actors.length; i++)
	{
		Actor *a = ListGet(l->actors, i);
		a->Update(a, delta);
	}

	if (IsKeyJustPressedPhys(SDL_SCANCODE_L))
	{
		Actor *leaf = CreateActor(state->level->player.pos, 0, 1, 0, 0, 0, 0, state->level->worldId);
		AddActor(leaf);
	}
	// if (IsKeyJustReleasedPhys(SDL_SCANCODE_L)) // CANNOT be else if because both of these can happen on the same tick
	// {
	// 	Actor *door = CreateActor(state->level->player.pos, 0, 4, 0, 0, 0, 0, state->level->worldId);
	// 	AddActor(door);
	// }

	targetedEnemy = GetTargetedEnemy(10);
	if (targetedEnemy)
	{
		if (IsMouseButtonJustPressedPhys(SDL_BUTTON_LEFT) || IsButtonJustPressedPhys(SDL_CONTROLLER_BUTTON_X))
		{
			RemoveActor(targetedEnemy);
		}
	}

	b2World_Step(l->worldId, (float)delta / PHYSICS_TARGET_TPS, 4);
	l->player.pos = b2Body_GetPosition(l->player.bodyId);
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void GMainStateRender(GlobalState *State)
{
	const Level *l = State->level;

	RenderLevel(State);

	SDL_Rect coinIconRect = {WindowWidth() - 260, 16, 40, 40};
	DrawTexture(v2(WindowWidthFloat() - 260, 16), v2(40, 40), TEXTURE("interface_hud_ycoin"));

	char coinStr[16];
	sprintf(coinStr, "%d", State->saveData->coins);
	FontDrawString(v2(WindowWidthFloat() - 210, 16), coinStr, 40, COLOR_WHITE, largeFont);

	coinIconRect.y = 64;

	for (int bc = 0; bc < State->saveData->blueCoins; bc++)
	{
		coinIconRect.x = WindowWidth() - 260 + bc * 48;
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

	if (State->textBoxActive)
	{
		TextBoxRender(&State->textBox, State->textBoxPage);
	}
	DPrintF("Position: (%.2f, %.2f)\nRotation: %.4f (%.2fdeg)",
			COLOR_WHITE,
			false,
			l->player.pos.x,
			l->player.pos.y,
			fabsf(l->player.angle),
			radToDeg(fabsf(l->player.angle)));

	DPrintF("Walls: %d", COLOR_WHITE, false, l->walls.length);
	DPrintF("Actors: %d", COLOR_WHITE, false, l->actors.length);
	DPrintF("Targeted Actor: %p", COLOR_WHITE, false, targetedEnemy);
}

void GMainStateSet()
{
	SetStateCallbacks(GMainStateUpdate, GMainStateFixedUpdate, MAIN_STATE, GMainStateRender);
}
