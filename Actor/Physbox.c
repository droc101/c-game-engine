//
// Created by droc101 on 4/28/25.
//

#include "Physbox.h"
#include <joltc/constants.h>
#include <joltc/enums.h>
#include <joltc/joltc.h>
#include <joltc/Math/Transform.h>
#include <joltc/Math/Vector3.h>
#include <joltc/Physics/Body/BodyCreationSettings.h>
#include <joltc/Physics/Body/BodyInterface.h>
#include <joltc/Physics/Body/MassProperties.h>
#include <joltc/Physics/Collision/Shape/Shape.h>
#include "../Helpers/Core/AssetLoaders/ModelLoader.h"
#include "../Helpers/Core/AssetReader.h"
#include "../Helpers/Core/KVList.h"
#include "../Helpers/Core/Physics/Physics.h"
#include "../Structs/Actor.h"

static inline void CreatePhysboxCollider(Actor *this, const Transform *transform)
{
	JPH_BodyCreationSettings *bodyCreationSettings = JPH_BodyCreationSettings_Create2_GAME(
			(const JPH_Shape *)JPH_BoxShape_Create((Vector3[]){{0.2f, 0.2f, 0.2f}}, JPH_DefaultConvexRadius),
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
	this->actorFlags = ACTOR_FLAG_CAN_BLOCK_LASERS | ACTOR_FLAG_CAN_BE_HELD;
	this->actorModel = LoadModel(MODEL("cube"));
	transform->position.y = -0.3f;

	CreatePhysboxCollider(this, transform);
}
