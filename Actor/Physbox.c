//
// Created by droc101 on 4/28/25.
//

#include "Physbox.h"
#include "../Helpers/Core/AssetReader.h"

void CreatePhysboxCollider(Actor *this, JPH_BodyInterface *bodyInterface)
{
	// b2BodyDef bodyDef = b2DefaultBodyDef();
	// bodyDef.type = b2_dynamicBody;
	// bodyDef.position = this->position;
	// bodyDef.linearDamping = 10;
	// bodyDef.fixedRotation = true;
	// this->bodyId = b2CreateBody(worldId, &bodyDef);
	//
	// const b2Polygon sensorShape = b2MakeOffsetBox(0.2f, 0.2f, (Vector2){0, 0}, 0);
	// b2ShapeDef shapeDef = b2DefaultShapeDef();
	// shapeDef.filter.categoryBits = COLLISION_GROUP_ACTOR;
	// b2CreatePolygonShape(this->bodyId, &shapeDef, &sensorShape);
}

void PhysboxInit(Actor *this, const KvList * /*params*/, JPH_BodyInterface *bodyInterface)
{
	CreatePhysboxCollider(this, bodyInterface);

	this->actorModel = LoadModel(MODEL("model_cube"));
	this->yPosition = -0.3f;
}

void PhysboxUpdate(Actor *this, const double)
{
	// this->position = b2Body_GetPosition(this->bodyId);
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void PhysboxDestroy(Actor *this)
{
	// b2DestroyBody(this->bodyId);
}
