//
// Created by droc101 on 4/28/25.
//

#include "actor/Physbox.h"
#include <engine/assets/AssetReader.h>
#include <engine/assets/ModelLoader.h>
#include <engine/physics/Physics.h>
#include <engine/structs/Actor.h>
#include <engine/structs/ActorDefinition.h>
#include <engine/structs/KVList.h>
#include <joltc/enums.h>
#include <joltc/Math/Transform.h>
#include <joltc/Physics/Body/BodyCreationSettings.h>
#include <joltc/Physics/Body/BodyInterface.h>
#include <joltc/Physics/Body/MassProperties.h>

static inline void CreatePhysboxCollider(Actor *this, const Transform *transform)
{
	JPH_BodyCreationSettings
			*bodyCreationSettings = JPH_BodyCreationSettings_Create2_GAME(this->actorModel->collisionModelShape,
																		  transform,
																		  JPH_MotionType_Dynamic,
																		  OBJECT_LAYER_DYNAMIC,
																		  this);
	const JPH_MassProperties massProperties = {
		.mass = 10.0f,
	};
	JPH_BodyCreationSettings_SetMassPropertiesOverride(bodyCreationSettings, &massProperties);
	JPH_BodyCreationSettings_SetOverrideMassProperties(bodyCreationSettings,
													   JPH_OverrideMassProperties_CalculateInertia);
	this->bodyId = JPH_BodyInterface_CreateAndAddBody(this->bodyInterface,
													  bodyCreationSettings,
													  JPH_Activation_Activate);
	JPH_BodyCreationSettings_Destroy(bodyCreationSettings);
}

void PhysboxInit(Actor *this, const KvList /*params*/, Transform *transform)
{
	this->actorFlags = ACTOR_FLAG_CAN_BLOCK_LASERS | ACTOR_FLAG_CAN_BE_HELD;
	this->actorModel = LoadModel(MODEL("cube"));

	CreatePhysboxCollider(this, transform);
}

ActorDefinition physboxActorDefinition = {
	.Update = DefaultActorUpdate,
	.OnPlayerContactAdded = DefaultActorOnPlayerContactAdded,
	.OnPlayerContactPersisted = DefaultActorOnPlayerContactPersisted,
	.OnPlayerContactRemoved = DefaultActorOnPlayerContactRemoved,
	.RenderUi = DefaultActorRenderUi,
	.Destroy = DefaultActorDestroy,
	.Init = PhysboxInit,
};

void RegisterPhysbox()
{
	RegisterDefaultActorInputs(&physboxActorDefinition);
	RegisterActor(PHYSBOX_ACTOR_NAME, &physboxActorDefinition);
}
