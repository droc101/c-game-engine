//
// Created by droc101 on 3/2/26.
//

#include <engine/actor/PhysicsModel.h>
#include <engine/physics/Physics.h>
#include <engine/structs/Actor.h>
#include <engine/subsystem/Logging.h>

static inline void CreatePhysicsModelCollider(Actor *this,
											  const Transform *transform,
											  const bool useAABB,
											  const float massOverride)
{
	JPH_BodyCreationSettings *bodyCreationSettings = NULL;
	if (!useAABB)
	{
		bodyCreationSettings = JPH_BodyCreationSettings_Create2_GAME(this->actorModel->collisionModelShape,
																	 transform,
																	 JPH_MotionType_Dynamic,
																	 OBJECT_LAYER_DYNAMIC,
																	 this);
	} else
	{
		bodyCreationSettings = JPH_BodyCreationSettings_Create2_GAME(this->actorModel->boundingBoxShape,
																	 transform,
																	 JPH_MotionType_Dynamic,
																	 OBJECT_LAYER_DYNAMIC,
																	 this);
		this->actorFlags = ACTOR_FLAG_USING_BOUNDING_BOX_COLLISION;
	}

	if (massOverride > 0)
	{
		const JPH_MassProperties massProperties = {
			.mass = massOverride,
		};
		JPH_BodyCreationSettings_SetMassPropertiesOverride(bodyCreationSettings, &massProperties);
		JPH_BodyCreationSettings_SetOverrideMassProperties(bodyCreationSettings,
														   JPH_OverrideMassProperties_CalculateInertia);
	}

	this->bodyId = JPH_BodyInterface_CreateAndAddBody(this->bodyInterface,
													  bodyCreationSettings,
													  JPH_Activation_Activate);
	JPH_BodyCreationSettings_Destroy(bodyCreationSettings);
}

void PhysicsModelInit(Actor *this, const KvList params, Transform *transform)
{
	this->actorModel = LoadModel(KvGetString(params, "model", "leafy"));
	this->currentSkinIndex = KvGetInt(params, "skin", 0);
	this->modColor = KvGetColor(params, "color", COLOR_WHITE);
	const float massOverride = KvGetFloat(params, "mass_override", 0.0f);
	if (this->actorModel->collisionModelType != COLLISION_MODEL_TYPE_DYNAMIC)
	{
		LogWarning("Tried to create a " PHYSICS_MODEL_ACTOR_NAME
				   " with full collision, but the model file (\"%s\") does not have any!\n",
				   KvGetString(params, "model", "model/quad.gmdl"));
		CreatePhysicsModelCollider(this, transform, true, massOverride);
	} else
	{
		CreatePhysicsModelCollider(this, transform, false, massOverride);
	}

	if (KvGetBool(params, "allow_pickup", true))
	{
		this->actorFlags |= ACTOR_FLAG_CAN_BE_HELD;
	}
}

static void PhysicsModelSetColorHandler(Actor *this, const Actor * /*sender*/, const Param *param)
{
	if (param->type == PARAM_TYPE_COLOR)
	{
		this->modColor = param->colorValue;
	}
}

static void PhysicsModelSetSkinHandler(Actor *this, const Actor * /*sender*/, const Param *param)
{
	uint32_t newSkin = this->currentSkinIndex;
	if (param->type == PARAM_TYPE_BYTE)
	{
		newSkin = param->byteValue;
	} else if (param->type == PARAM_TYPE_INTEGER)
	{
		newSkin = param->intValue;
	} else if (param->type == PARAM_TYPE_UINT_64)
	{
		newSkin = param->uint64value;
	}
	if (this->actorModel->skinCount <= newSkin)
	{
		LogError("Tried to switch to skin %u, but that index is out of bounds\n", newSkin);
	} else
	{
		this->currentSkinIndex = newSkin;
	}
}

ActorDefinition physicsModelActorDefinition = {
	.Update = DefaultActorUpdate,
	.OnPlayerContactAdded = DefaultActorOnPlayerContactAdded,
	.OnPlayerContactPersisted = DefaultActorOnPlayerContactPersisted,
	.OnPlayerContactRemoved = DefaultActorOnPlayerContactRemoved,
	.RenderUi = DefaultActorRenderUi,
	.Destroy = DefaultActorDestroy,
	.Init = PhysicsModelInit,
};

void RegisterPhysicsModel()
{
	RegisterDefaultActorInputs(&physicsModelActorDefinition);
	RegisterActorInput(&physicsModelActorDefinition, PHYSICS_MODEL_INPUT_SET_COLOR, PhysicsModelSetColorHandler);
	RegisterActorInput(&physicsModelActorDefinition, PHYSICS_MODEL_INPUT_SET_SKIN, PhysicsModelSetSkinHandler);
	RegisterActor(PHYSICS_MODEL_ACTOR_NAME, &physicsModelActorDefinition);
}
