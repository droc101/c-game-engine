//
// Created by droc101 on 4/28/25.
//

#include "Physbox.h"
#include "../Helpers/Core/AssetReader.h"
#include "../Helpers/Core/AssetLoaders/ModelLoader.h"

void CreatePhysboxCollider(Actor *this, const Transform *transform)
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

	JPH_BodyCreationSettings *bodyCreationSettings = JPH_BodyCreationSettings_Create2_GAME(
			(const JPH_Shape *)JPH_BoxShape_Create((Vector3[]){{0.2f, 0.2f, 0.2f}}, JPH_DEFAULT_CONVEX_RADIUS),
			transform,
			JPH_MotionType_Dynamic,
			OBJECT_LAYER_DYNAMIC,
			this);
	const JPH_MassProperties massProperties = {
		.mass = 2.0f,
	};
	JPH_BodyCreationSettings_SetMassPropertiesOverride(bodyCreationSettings, &massProperties);
	JPH_BodyCreationSettings_SetOverrideMassProperties(bodyCreationSettings,
													   JPH_OverrideMassProperties_CalculateInertia);
	JPH_BodyCreationSettings_SetLinearDamping(bodyCreationSettings, 10.0f);
	JPH_BodyCreationSettings_SetAngularDamping(bodyCreationSettings, 5.0f);
	this->bodyId = JPH_BodyInterface_CreateAndAddBody(this->bodyInterface,
													  bodyCreationSettings,
													  JPH_Activation_Activate);
	JPH_BodyCreationSettings_Destroy(bodyCreationSettings);
}

void PhysboxInit(Actor *this, const KvList * /*params*/, Transform *transform)
{
	this->actorFlags = ACTOR_FLAG_CAN_BLOCK_LASERS;
	this->actorModel = LoadModel(MODEL("cube"));
	transform->position.y = -0.3f;

	CreatePhysboxCollider(this, transform);
}
