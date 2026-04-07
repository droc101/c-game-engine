//
// Created by droc101 on 4/3/26.
//

#include "actor/NpcJohn.h"
#include <cglm/types.h>
#include <engine/assets/AssetReader.h>
#include <engine/physics/Physics.h>
#include <engine/structs/Actor.h>
#include <engine/structs/ActorDefinition.h>
#include <engine/structs/ActorWall.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/KVList.h>
#include <engine/structs/Map.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Error.h>
#include <joltc/constants.h>
#include <joltc/enums.h>
#include <joltc/joltc.h>
#include <joltc/Math/Transform.h>
#include <joltc/Math/Vector3.h>
#include <joltc/Physics/Body/BodyCreationSettings.h>
#include <joltc/Physics/Body/BodyID.h>
#include <joltc/Physics/Body/BodyInterface.h>
#include <joltc/Physics/Collision/Shape/Shape.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

static inline void CreateNpcJohnCollider(Actor *this, const Transform *transform)
{
	JPH_Shape *shape = (JPH_Shape *)JPH_CylinderShape_Create(0.5f, 0.25f);

	JPH_BodyCreationSettings *bodyCreationSettings = JPH_BodyCreationSettings_Create2_GAME(shape,
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
	JPH_BodyCreationSettings_SetAllowedDOFs(bodyCreationSettings,
											JPH_AllowedDOFs_TranslationX |
													JPH_AllowedDOFs_TranslationY |
													JPH_AllowedDOFs_TranslationZ |
													JPH_AllowedDOFs_RotationY);
	this->bodyId = JPH_BodyInterface_CreateAndAddBody(this->bodyInterface,
													  bodyCreationSettings,
													  JPH_Activation_Activate);
	JPH_BodyCreationSettings_Destroy(bodyCreationSettings);
	JPH_Shape_Destroy(shape);
}

static void JohnUpdate(Actor *this, double /*delta*/)
{
	Vector3 position = {};
	JPH_BodyInterface_GetPosition(this->bodyInterface, this->bodyId, &position);
	const float rotation = atan2f(GetState()->camera->transform.position.z - position.z,
								  GetState()->camera->transform.position.x - position.x) +
						   GLM_PI_2f;
	this->wall->a = v2(0.5f * cosf(rotation), 0.5f * sinf(rotation));
	this->wall->b = v2(-0.5f * cosf(rotation), -0.5f * sinf(rotation));
	ActorWallBake(this);
}

void JohnInit(Actor *this, const KvList /*params*/, Transform *transform)
{
	this->wall = malloc(sizeof(ActorWall));
	CheckAlloc(this->wall);
	this->wall->a = v2(0, 0.5f);
	this->wall->b = v2(0, -0.5f);
	this->wall->tex = malloc(strlen(TEXTURE("actor/john")) + 1);
	strcpy(this->wall->tex, TEXTURE("actor/john"));
	this->wall->uvScale = v2s(1.0f);
	this->wall->uvOffset = v2s(0.0f);
	this->wall->height = 1.0f;
	this->wall->unshaded = false;
	ActorWallBake(this);
	CreateNpcJohnCollider(this, transform);
}

ActorDefinition npcJohnActorDefinition = {
	.Update = JohnUpdate,
	.OnPlayerContactAdded = DefaultActorOnPlayerContactAdded,
	.OnPlayerContactPersisted = DefaultActorOnPlayerContactPersisted,
	.OnPlayerContactRemoved = DefaultActorOnPlayerContactRemoved,
	.RenderUi = DefaultActorRenderUi,
	.Interact = DefaultActorInteract,
	.Destroy = DefaultActorDestroy,
	.Init = JohnInit,
};

void RegisterNpcJohn()
{
	RegisterDefaultActorInputs(&npcJohnActorDefinition);
	RegisterActor(NPC_JOHN_ACTOR_NAME, &npcJohnActorDefinition);
}
