//
// Created by droc101 on 4/22/2024.
//

#include "TestActor.h"
#include <joltc/enums.h>
#include <joltc/joltc.h>
#include <joltc/Math/Transform.h>
#include <joltc/Physics/Body/BodyCreationSettings.h>
#include <joltc/Physics/Body/BodyInterface.h>
#include <joltc/Physics/Body/MassProperties.h>
#include <joltc/Physics/Collision/Shape/Shape.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "../Helpers/Core/AssetLoaders/ModelLoader.h"
#include "../Helpers/Core/AssetReader.h"
#include "../Helpers/Core/Error.h"
#include "../Helpers/Core/KVList.h"
#include "../Helpers/Core/Logging.h"
#include "../Helpers/Core/MathEx.h"
#include "../Helpers/Core/Physics/Navigation.h"
#include "../Helpers/Core/Physics/Physics.h"
#include "../Helpers/Graphics/Font.h"
#include "../Helpers/Graphics/RenderingHelpers.h"
#include "../Structs/Actor.h"
#include "../Structs/Color.h"
#include "../Structs/GlobalState.h"
#include "../Structs/Vector2.h"

static inline void CreateTestActorCollider(Actor *this, const Transform *transform)
{
	const JPH_Shape *shape = (const JPH_Shape *)JPH_CapsuleShape_Create(0.25f, 0.2867f);
	JPH_BodyCreationSettings *bodyCreationSettings = JPH_BodyCreationSettings_Create2_GAME(shape,
																						   transform,
																						   JPH_MotionType_Dynamic,
																						   OBJECT_LAYER_DYNAMIC,
																						   this);
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

static bool TestActorSignalHandler(Actor *this, const Actor *sender, const uint8_t signal, const Param *param)
{
	if (DefaultSignalHandler(this, sender, signal, param))
	{
		return true;
	}
	LogDebug("Test actor got signal %d from actor %p\n", signal, sender);
	return false;
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

static void TestActorUIRender(Actor *this)
{
	DrawTextAligned("I'm TestActor!",
					16,
					COLOR_BLACK,
					v2s(22),
					v2(WindowWidth() - 40, WindowHeight() - 40),
					FONT_HALIGN_CENTER,
					FONT_VALIGN_BOTTOM,
					smallFont);
	DrawTextAligned("I'm TestActor!",
					16,
					COLOR_WHITE,
					v2s(20),
					v2(WindowWidth() - 40, WindowHeight() - 40),
					FONT_HALIGN_CENTER,
					FONT_VALIGN_BOTTOM,
					smallFont);
	if (!GetState()->level->player.hasHeldActor && GetState()->level->player.targetedActor == this)
	{
		DrawTextAligned("please spare me",
						16,
						COLOR_BLACK,
						v2(22, 102),
						v2(WindowWidth() - 40, WindowHeight() - 40),
						FONT_HALIGN_CENTER,
						FONT_VALIGN_MIDDLE,
						smallFont);
		DrawTextAligned("please spare me",
						16,
						COLOR_WHITE,
						v2(20, 100),
						v2(WindowWidth() - 40, WindowHeight() - 40),
						FONT_HALIGN_CENTER,
						FONT_VALIGN_MIDDLE,
						smallFont);
	}
}

void TestActorInit(Actor *this, const KvList * /*params*/, Transform *transform)
{
	this->Update = TestActorUpdate;
	this->SignalHandler = TestActorSignalHandler;
	this->Render = TestActorUIRender;

	this->actorFlags = ACTOR_FLAG_ENEMY;
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
