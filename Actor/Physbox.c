//
// Created by droc101 on 4/28/25.
//

#include "Physbox.h"
#include "../Helpers/Core/AssetReader.h"

void CreatePhysboxCollider(Actor *this)
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

	JPH_BodyCreationSettings *bodyCreationSettings = JPH_BodyCreationSettings_Create3(
			(const JPH_Shape *)JPH_BoxShape_Create((Vector3[]){{0.2f, 0.2f, 0.2f}}, JPH_DEFAULT_CONVEX_RADIUS),
			&this->transform.position,
			NULL,
			JPH_MotionType_Dynamic,
			OBJECT_LAYER_DYNAMIC);
	const JPH_MassProperties massProperties = {
		.mass = 2.0f,
	};
	JPH_BodyCreationSettings_SetMassPropertiesOverride(bodyCreationSettings, &massProperties);
	JPH_BodyCreationSettings_SetOverrideMassProperties(bodyCreationSettings,
													   JPH_OverrideMassProperties_CalculateInertia);
	JPH_BodyCreationSettings_SetLinearDamping(bodyCreationSettings, 10.0f);
	JPH_BodyCreationSettings_SetAngularDamping(bodyCreationSettings, 5.0f);
	JPH_BodyCreationSettings_SetUserData(bodyCreationSettings, (uint64_t)this);
	this->bodyId = JPH_BodyInterface_CreateAndAddBody(this->bodyInterface,
													  bodyCreationSettings,
													  JPH_Activation_Activate);
}

void PhysboxInit(Actor *this, const KvList * /*params*/)
{
	this->actorModel = LoadModel(MODEL("model_cube"));
	this->transform.position.y = -0.3f;

	CreatePhysboxCollider(this);
}

void PhysboxUpdate(Actor *this, const double /*delta*/)
{
	JPH_Quat rotation;
	JPH_BodyInterface_GetPositionAndRotation(this->bodyInterface, this->bodyId, &this->transform.position, &rotation);
	JPH_Quat_GetEulerAngles(&rotation, &this->transform.rotation);
}
