//
// Created by droc101 on 4/22/2024.
//

#include "actor/TestActor.h"
#include <engine/assets/AssetReader.h>
#include <engine/assets/ModelLoader.h>
#include <engine/graphics/Font.h>
#include <engine/graphics/RenderingHelpers.h>
#include <engine/helpers/MathEx.h>
#include <engine/physics/Navigation.h>
#include <engine/physics/Physics.h>
#include <engine/structs/Actor.h>
#include <engine/structs/ActorDefinition.h>
#include <engine/structs/Color.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/KVList.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Error.h>
#include <joltc/enums.h>
#include <joltc/Math/Transform.h>
#include <joltc/Physics/Body/BodyCreationSettings.h>
#include <joltc/Physics/Body/BodyInterface.h>
#include <joltc/Physics/Body/MassProperties.h>
#include <stdlib.h>

static inline void CreateTestActorCollider(Actor *this, const Transform *transform)
{
	JPH_BodyCreationSettings
			*bodyCreationSettings = JPH_BodyCreationSettings_Create2_GAME(this->actorModel->collisionModelShape,
																		  transform,
																		  JPH_MotionType_Dynamic,
																		  OBJECT_LAYER_DYNAMIC,
																		  this);
	const JPH_MassProperties massProperties = {
		.mass = 15.0f,
	};
	JPH_BodyCreationSettings_SetMassPropertiesOverride(bodyCreationSettings, &massProperties);
	JPH_BodyCreationSettings_SetOverrideMassProperties(bodyCreationSettings,
													   JPH_OverrideMassProperties_CalculateInertia);
	JPH_BodyCreationSettings_SetAllowedDOFs(bodyCreationSettings, 0);
	// JPH_AllowedDOFs_TranslationX |
	// 		JPH_AllowedDOFs_TranslationY |
	// 		JPH_AllowedDOFs_TranslationZ |
	// 		JPH_AllowedDOFs_RotationY);
	this->bodyId = JPH_BodyInterface_CreateAndAddBody(this->bodyInterface,
													  bodyCreationSettings,
													  JPH_Activation_Activate);
	JPH_BodyCreationSettings_Destroy(bodyCreationSettings);
}

static void TestActorUpdate(Actor *this, const double delta)
{
	(void)this;
	(void)delta;
	// this->modColor.r = sinf(GetState()->physicsFrame / 10.0f) + 1.0f / 2.0f;
	// JPH_Quat rotation;
	// JPH_BodyInterface_GetPositionAndRotation(this->bodyInterface, this->bodyId, &this->transform.position, &rotation);
	// JPH_Quat_GetEulerAngles(&rotation, &this->transform.rotation);
	//
	// NavigationStep(this, this->extraData, delta);
}

static void TestActorRenderUi(Actor *this)
{
	DrawTextAligned("I'm TestActor!",
					16,
					COLOR_BLACK,
					v2s(22),
					v2(ScaledWindowWidth() - 40, ScaledWindowHeight() - 40),
					FONT_HALIGN_CENTER,
					FONT_VALIGN_BOTTOM,
					smallFont);
	DrawTextAligned("I'm TestActor!",
					16,
					COLOR_WHITE,
					v2s(20),
					v2(ScaledWindowWidth() - 40, ScaledWindowHeight() - 40),
					FONT_HALIGN_CENTER,
					FONT_VALIGN_BOTTOM,
					smallFont);
	if (!GetState()->map->player.hasHeldActor && GetState()->map->player.targetedActor == this)
	{
		DrawTextAligned("please spare me",
						16,
						COLOR_BLACK,
						v2(22, 102),
						v2(ScaledWindowWidth() - 40, ScaledWindowHeight() - 40),
						FONT_HALIGN_CENTER,
						FONT_VALIGN_MIDDLE,
						smallFont);
		DrawTextAligned("please spare me",
						16,
						COLOR_WHITE,
						v2(20, 100),
						v2(ScaledWindowWidth() - 40, ScaledWindowHeight() - 40),
						FONT_HALIGN_CENTER,
						FONT_VALIGN_MIDDLE,
						smallFont);
	}
}

static void TestActorIdle(Actor *this, const double /*delta*/)
{
	(void)this;
	// const NavigationConfig *navigationConfig = this->extraData;
	// this->transform.rotation.y += 0.01f;
	// const Vector2 impulse = v2(0, navigationConfig->speed * (float)delta);
	// b2Body_ApplyLinearImpulseToCenter(this->bodyId, Vector2Rotate(impulse, this->rotation), true);
}

static void TestActorTargetReached(Actor *this, const double delta)
{
	(void)this;
	(void)delta;
	// const NavigationConfig *navigationConfig = this->extraData;
	// this->transform.rotation.y += lerp(0, PlayerRelativeAngle(this), navigationConfig->rotationSpeed * (float)delta);
}

void TestActorInit(Actor *this, const KvList /*params*/, Transform *transform)
{
	this->actorFlags = ACTOR_FLAG_CAN_PUSH_PLAYER | ACTOR_FLAG_ENEMY;
	this->actorModel = LoadModel(MODEL("leafy"));
	CreateTestActorCollider(this, transform);

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

static ActorDefinition definition = {
	.actorType = ACTOR_TYPE_TEST_ACTOR,
	.Update = TestActorUpdate,
	.OnPlayerContactAdded = DefaultActorOnPlayerContactAdded,
	.OnPlayerContactPersisted = DefaultActorOnPlayerContactPersisted,
	.OnPlayerContactRemoved = DefaultActorOnPlayerContactRemoved,
	.RenderUi = TestActorRenderUi,
	.Destroy = DefaultActorDestroy,
	.Init = TestActorInit,
};

void RegisterTestActor()
{
	RegisterDefaultActorInputs(&definition);
	RegisterActor(TEST_ACTOR_NAME, &definition);
}
