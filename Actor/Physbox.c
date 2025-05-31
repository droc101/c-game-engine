//
// Created by droc101 on 4/28/25.
//

#include "Physbox.h"
#include "../Helpers/Core/AssetReader.h"
#include "box2d/box2d.h"

void CreatePhysboxCollider(Actor *this, const b2WorldId worldId)
{
	b2BodyDef bodyDef = b2DefaultBodyDef();
	bodyDef.type = b2_dynamicBody;
	bodyDef.position = this->position;
	bodyDef.linearDamping = 10;
	bodyDef.fixedRotation = true;
	this->bodyId = b2CreateBody(worldId, &bodyDef);

	const b2Polygon sensorShape = b2MakeOffsetBox(0.2f, 0.2f, (Vector2){0, 0}, 0);
	b2ShapeDef shapeDef = b2DefaultShapeDef();
	shapeDef.filter.categoryBits = COLLISION_GROUP_ACTOR;
	b2CreatePolygonShape(this->bodyId, &shapeDef, &sensorShape);
}

void PhysboxInit(Actor *this, const b2WorldId worldId, KvList *params)
{
	CreatePhysboxCollider(this, worldId);

	this->actorModel = LoadModel(MODEL("model_cube"));
	this->yPosition = -0.3f;
	this->showShadow = false;
}

void PhysboxUpdate(Actor *this, const double)
{
	this->position = b2Body_GetPosition(this->bodyId);
	// const b2Rot r = b2Body_GetRotation(this->bodyId);
	// this->rotation = b2Rot_GetAngle(r);
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void PhysboxDestroy(Actor *this)
{
	b2DestroyBody(this->bodyId);
}
