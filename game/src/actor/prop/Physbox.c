//
// Created by droc101 on 4/28/25.
//

#include "actor/prop/Physbox.h"
#include <engine/assets/AssetReader.h>
#include <engine/assets/ModelLoader.h>
#include <engine/physics/Physics.h>
#include <engine/structs/Actor.h>
#include <engine/structs/ActorDefinition.h>
#include <engine/structs/KVList.h>
#include <joltc/enums.h>
#include <joltc/joltc.h>
#include <joltc/Math/Quat.h>
#include <joltc/Math/Transform.h>
#include <joltc/Math/Vector3.h>
#include <joltc/Physics/Body/BodyCreationSettings.h>
#include <joltc/Physics/Body/BodyInterface.h>
#include <joltc/Physics/Body/MassProperties.h>
#include <joltc/Physics/Collision/Shape/Shape.h>
#include <stdbool.h>
#include <stdint.h>

static inline void CreatePhysboxCollider(Actor *this, const Transform *transform)
{
	(void)JPH_MutableCompoundShape_AddShape(this->shape,
											&Vector3_Zero,
											&JPH_Quat_Identity,
											this->model->collisionModelShape,
											0,
											UINT32_MAX);

	JPH_BodyCreationSettings *bodyCreationSettings = JPH_BodyCreationSettings_Create2_GAME((JPH_Shape *)this->shape,
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

static void PhysboxInit(Actor *this, const KvList /*params*/, const Transform *transform)
{
	this->flags = ACTOR_FLAG_CAN_BLOCK_LASERS | ACTOR_FLAG_CAN_BE_HELD;
	this->hasModel = true;
	this->model = LoadModel(MODEL("cube"));

	CreatePhysboxCollider(this, transform);
}

ActorDefinition physboxActorDefinition = {
	.Init = PhysboxInit,
};

void RegisterPhysbox()
{
	RegisterDefaultActorInputs(&physboxActorDefinition);
	RegisterActor(PHYSBOX_ACTOR_NAME, &physboxActorDefinition);
}
