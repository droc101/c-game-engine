//
// Created by droc101 on 7/12/2024.
//

#include "Goal.h"
#include <box2d/box2d.h>
#include <math.h>

#include "../Helpers/Collision.h"
#include "../Helpers/Core/AssetReader.h"
#include "../Helpers/Core/Error.h"
#include "../Helpers/Core/MathEx.h"
#include "../Helpers/TextBox.h"
#include "../Structs/Actor.h"
#include "../Structs/GlobalState.h"
#include "../Structs/Level.h"
#include "../Structs/Vector2.h"
#include "../Structs/Wall.h"

#define GOAL_OUTPUT_COLLECTED 2

void CreateGoalSensor(Actor *this, const b2WorldId worldId)
{
	this->extraData = calloc(1, sizeof(b2ShapeId));
	CheckAlloc(this->extraData);
	b2ShapeId *shapeId = this->extraData;

	b2BodyDef sensorBodyDef = b2DefaultBodyDef();
	sensorBodyDef.type = b2_staticBody;
	sensorBodyDef.position = this->position;
	this->bodyId = b2CreateBody(worldId, &sensorBodyDef);
	this->actorWall->bodyId = this->bodyId;
	const b2Circle sensorShape = {
		.radius = 0.5f,
	};
	b2ShapeDef sensorShapeDef = b2DefaultShapeDef();
	sensorShapeDef.isSensor = true;
	sensorShapeDef.filter.categoryBits = COLLISION_GROUP_ACTOR;
	sensorShapeDef.filter.maskBits = COLLISION_GROUP_PLAYER;
	*shapeId = b2CreateCircleShape(this->bodyId, &sensorShapeDef, &sensorShape);
}

void GoalInit(Actor *this, const b2WorldId worldId, KvList *params)
{
	this->actorWall = CreateWall(v2(0, 0.5f), v2(0, -0.5f), TEXTURE("actor_goal0"), 1.0f, 0.0f);
	WallBake(this->actorWall);

	CreateGoalSensor(this, worldId);
}

void GoalUpdate(Actor *this, double /*delta*/)
{
	const Vector2 playerPosition = GetState()->level->player.pos;
	const float rotation = atan2f(playerPosition.y - this->position.y, playerPosition.x - this->position.x) + PIf / 2;
	this->actorWall->a = v2(0.5f * cosf(rotation), 0.5f * sinf(rotation));
	this->actorWall->b = v2(-0.5f * cosf(rotation), -0.5f * sinf(rotation));

	if (GetSensorState(GetState()->level->worldId, ((b2ShapeId *)this->extraData)->index1, false))
	{
		const TextBox tb = DEFINE_TEXT("Goal!",
									   2,
									   20,
									   0,
									   70,
									   TEXT_BOX_H_ALIGN_CENTER,
									   TEXT_BOX_V_ALIGN_TOP,
									   TEXT_BOX_THEME_WHITE);
		ShowTextBox(tb);
		ActorFireOutput(this, GOAL_OUTPUT_COLLECTED, PARAM_NONE);
		RemoveActor(this);
	}
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void GoalDestroy(Actor *this)
{
	b2DestroyBody(this->bodyId);
	free(this->actorWall);
	free(this->extraData);
}
