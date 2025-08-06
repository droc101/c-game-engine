//
// Created by droc101 on 4/22/2024.
//

#include "TestActor.h"
#include "../Helpers/Core/AssetReader.h"
#include "../Helpers/Core/Error.h"
#include "../Helpers/Core/Logging.h"
#include "../Helpers/Core/MathEx.h"
#include "../Helpers/Core/Physics/Navigation.h"
#include "../Structs/Actor.h"

bool TestActorSignalHandler(Actor *this, const Actor *sender, const byte signal, const Param *param)
{
	if (DefaultSignalHandler(this, sender, signal, param))
	{
		return true;
	}
	LogDebug("Test actor got signal %d from actor %p\n", signal, sender);
	return false;
}

void TestActorIdle(Actor *this, const double /*delta*/)
{
	(void)this;
	// const NavigationConfig *navigationConfig = this->extraData;
	// this->transform.rotation.y += 0.01f;
	// const Vector2 impulse = v2(0, navigationConfig->speed * (float)delta);
	// b2Body_ApplyLinearImpulseToCenter(this->bodyId, Vector2Rotate(impulse, this->rotation), true);
}

void TestActorTargetReached(Actor *this, const double delta)
{
	(void)this;
	(void)delta;
	// const NavigationConfig *navigationConfig = this->extraData;
	// this->transform.rotation.y += lerp(0, PlayerRelativeAngle(this), navigationConfig->rotationSpeed * (float)delta);
}

void CreateTestActorCollider(Actor *this, const Transform *transform)
{
	const JPH_Shape *shape = (const JPH_Shape *)JPH_CapsuleShape_Create(0.25f, 0.2867f);
	JPH_BodyCreationSettings *bodyCreationSettings = JPH_BodyCreationSettings_Create3(shape,
																					  &transform->position,
																					  &JPH_Quat_Identity,
																					  JPH_MotionType_Dynamic,
																					  OBJECT_LAYER_DYNAMIC);
	const JPH_MassProperties massProperties = {
		.mass = 20.0f,
	};
	JPH_BodyCreationSettings_SetMassPropertiesOverride(bodyCreationSettings, &massProperties);
	JPH_BodyCreationSettings_SetOverrideMassProperties(bodyCreationSettings,
													   JPH_OverrideMassProperties_CalculateInertia);
	JPH_BodyCreationSettings_SetLinearDamping(bodyCreationSettings, 10.0f);
	JPH_BodyCreationSettings_SetAngularDamping(bodyCreationSettings, 5.0f);
	JPH_BodyCreationSettings_SetAllowedDOFs(bodyCreationSettings,
											JPH_AllowedDOFs_TranslationX |
													JPH_AllowedDOFs_TranslationY |
													JPH_AllowedDOFs_TranslationZ |
													JPH_AllowedDOFs_RotationY);
	JPH_BodyCreationSettings_SetUserData(bodyCreationSettings, (uint64_t)this);
	this->bodyId = JPH_BodyInterface_CreateAndAddBody(this->bodyInterface,
													  bodyCreationSettings,
													  JPH_Activation_Activate);
	JPH_BodyCreationSettings_Destroy(bodyCreationSettings);
}

void TestActorInit(Actor *this, const KvList * /*params*/, Transform *transform)
{
	CreateTestActorCollider(this, transform);

	this->actorFlags |= ACTOR_FLAG_ENEMY;

	this->actorModel = LoadModel(MODEL("model_leafy"));
	this->currentSkinIndex = 0;
	this->SignalHandler = TestActorSignalHandler;
	this->extraData = calloc(1, sizeof(NavigationConfig));
	CheckAlloc(this->extraData);
	NavigationConfig *navigationConfig = this->extraData;
	navigationConfig->fov = PIf / 2;
	navigationConfig->speed = 0.075f;
	navigationConfig->rotationSpeed = 0.1f;
	navigationConfig->directness = 0.5f;
	navigationConfig->minDistance = 1.5f;
	navigationConfig->agroDistance = 10;
	navigationConfig->deAgroDistance = 20;
	navigationConfig->agroTicks = 120;
	navigationConfig->IdleFunction = TestActorIdle;
	navigationConfig->TargetReachedFunction = TestActorTargetReached;
	navigationConfig->lastKnownTarget.x = transform->position.x;
	navigationConfig->lastKnownTarget.y = transform->position.z;
}

void TestActorUpdate(Actor *this, const double delta)
{
	(void)this;
	(void)delta;
	// JPH_Quat rotation;
	// JPH_BodyInterface_GetPositionAndRotation(this->bodyInterface, this->bodyId, &this->transform.position, &rotation);
	// JPH_Quat_GetEulerAngles(&rotation, &this->transform.rotation);
	//
	// NavigationStep(this, this->extraData, delta);
}
