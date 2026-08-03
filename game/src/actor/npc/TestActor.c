//
// Created by droc101 on 4/22/2024.
//

#include "actor/npc/TestActor.h"
#include <engine/assets/AssetReader.h>
#include <engine/assets/ModelLoader.h>
#include <engine/graphics/Font.h>
#include <engine/graphics/RenderingHelpers.h>
#include <engine/physics/Physics.h>
#include <engine/structs/Actor.h>
#include <engine/structs/ActorDefinition.h>
#include <engine/structs/Color.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/KVList.h>
#include <engine/structs/Vector2.h>
#include <joltc/enums.h>
#include <joltc/Math/Transform.h>
#include <joltc/Physics/Body/BodyCreationSettings.h>
#include <joltc/Physics/Body/BodyInterface.h>
#include <joltc/Physics/Body/MassProperties.h>
#include <stdbool.h>

static inline void CreateTestActorCollider(Actor *this, const Transform *transform)
{
	JPH_BodyCreationSettings
			*bodyCreationSettings = JPH_BodyCreationSettings_Create2_GAME(this->model->collisionModelShape,
																		  transform,
																		  JPH_MotionType_Dynamic,
																		  OBJECT_LAYER_DYNAMIC,
																		  this);
	const JPH_MassProperties massProperties = {
		.mass = 80.0f,
	};
	JPH_BodyCreationSettings_SetMassPropertiesOverride(bodyCreationSettings, &massProperties);
	JPH_BodyCreationSettings_SetOverrideMassProperties(bodyCreationSettings,
													   JPH_OverrideMassProperties_CalculateInertia);
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
}

static void TestActorRenderUi(Actor *this)
{
	if (GetState()->camera == &GetState()->map->player.playerCamera)
	{
		DrawTextAligned("I'm TestActor!",
						16,
						COLOR_BLACK,
						v2s(22),
						v2(ScaledWindowWidth() - 40, ScaledWindowHeight() - 40),
						FONT_HALIGN_CENTER,
						FONT_VALIGN_BOTTOM,
						FONT("small_font"));
		DrawTextAligned("I'm TestActor!",
						16,
						COLOR_WHITE,
						v2s(20),
						v2(ScaledWindowWidth() - 40, ScaledWindowHeight() - 40),
						FONT_HALIGN_CENTER,
						FONT_VALIGN_BOTTOM,
						FONT("small_font"));
	}

	if (!GetState()->map->player.hasHeldActor && GetState()->map->player.targetedActor == this)
	{
		DrawTextAligned("please spare me",
						16,
						COLOR_BLACK,
						v2(22, 102),
						v2(ScaledWindowWidth() - 40, ScaledWindowHeight() - 40),
						FONT_HALIGN_CENTER,
						FONT_VALIGN_MIDDLE,
						FONT("small_font"));
		DrawTextAligned("please spare me",
						16,
						COLOR_WHITE,
						v2(20, 100),
						v2(ScaledWindowWidth() - 40, ScaledWindowHeight() - 40),
						FONT_HALIGN_CENTER,
						FONT_VALIGN_MIDDLE,
						FONT("small_font"));
	}
}

static void TestActorInit(Actor *this, const KvList /*params*/, Transform *transform)
{
	this->flags = ACTOR_FLAG_CAN_PUSH_PLAYER | ACTOR_FLAG_ENEMY;
	this->hasModel = true;
	this->model = LoadModel(MODEL("leafy"));
	CreateTestActorCollider(this, transform);
}

ActorDefinition testActorDefinition = {
	.Update = TestActorUpdate,
	.OnPlayerContactAdded = DefaultActorOnPlayerContactAdded,
	.OnPlayerContactPersisted = DefaultActorOnPlayerContactPersisted,
	.OnPlayerContactRemoved = DefaultActorOnPlayerContactRemoved,
	.RenderUi = TestActorRenderUi,
	.Interact = DefaultActorInteract,
	.Destroy = DefaultActorDestroy,
	.Init = TestActorInit,
};

void RegisterTestActor()
{
	RegisterDefaultActorInputs(&testActorDefinition);
	RegisterActor(TEST_ACTOR_NAME, &testActorDefinition);
}
