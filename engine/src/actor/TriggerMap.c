//
// Created by droc101 on 6/28/26.
//

#include <engine/actor/TriggerMap.h>
#include <engine/gameState/LoadingState.h>
#include <engine/physics/Physics.h>
#include <engine/structs/Actor.h>
#include <engine/structs/ActorDefinition.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/KVList.h>
#include <engine/structs/Map.h>
#include <engine/subsystem/Error.h>
#include <joltc/constants.h>
#include <joltc/enums.h>
#include <joltc/joltc.h>
#include <joltc/Math/Quat.h>
#include <joltc/Math/RVec3.h>
#include <joltc/Math/Transform.h>
#include <joltc/Math/Vector3.h>
#include <joltc/Physics/Body/BodyCreationSettings.h>
#include <joltc/Physics/Body/BodyID.h>
#include <joltc/Physics/Body/BodyInterface.h>
#include <joltc/Physics/Collision/Shape/Shape.h>
#include <SDL3/SDL_stdinc.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef struct TriggerMapData
{
	float width;
	float height;
	float depth;
	bool enabled;
	char *mapName;
	char *entranceName;
	Transform xfm;
} TriggerMapData;

static inline void CreateTriggerMapSensor(Actor *this, const Transform *transform)
{
	const TriggerMapData *data = this->extraData;
	JPH_Shape *shape = (JPH_Shape *)
			JPH_BoxShape_Create((Vector3[]){{data->width / 2, data->height / 2, data->depth / 2}},
								JPH_DefaultConvexRadius);
	JPH_BodyCreationSettings *bodyCreationSettings = JPH_BodyCreationSettings_Create2_GAME(shape,
																						   transform,
																						   JPH_MotionType_Static,
																						   OBJECT_LAYER_SENSOR,
																						   this);
	JPH_BodyCreationSettings_SetIsSensor(bodyCreationSettings, true);
	this->bodyId = JPH_BodyInterface_CreateAndAddBody(this->bodyInterface,
													  bodyCreationSettings,
													  JPH_Activation_Activate);
	JPH_Shape_Destroy(shape);
	JPH_BodyCreationSettings_Destroy(bodyCreationSettings);
}

static void TriggerMapChange(const TriggerMapData *data)
{
	MapTransition *transition = malloc(sizeof(MapTransition));
	CheckAlloc(transition);
	transition->entranceName = strdup(data->entranceName);

	Vector3 thisRot;
	JPH_Quat_GetEulerAngles(&data->xfm.rotation, &thisRot);
	Vector3 playerRot;
	JPH_Quat_GetEulerAngles(&GetState()->map->player.transform.rotation, &playerRot);

	JPH_RVec3_Subtract(&playerRot, &thisRot, &transition->relativeAngles);

	JPH_RVec3_Subtract(&GetState()->map->player.transform.position, &data->xfm.position, &transition->relativePosition);

	loadStateTransition = transition;
	loadStateLevelname = strdup(data->mapName);
	SetGameState(&LoadingState);
}

static void TriggerForceTriggerHandler(Actor *this, const Actor * /*sender*/, const Param * /*param*/)
{
	TriggerMapChange((TriggerMapData *)this->extraData);
}

static void TriggerEnableHandler(Actor *this, const Actor * /*sender*/, const Param * /*param*/)
{
	TriggerMapData *data = (TriggerMapData *)this->extraData;
	data->enabled = true;
}

static void TriggerDisableHandler(Actor *this, const Actor * /*sender*/, const Param * /*param*/)
{
	TriggerMapData *data = (TriggerMapData *)this->extraData;
	data->enabled = false;
}

static void TriggerMapOnPlayerContactPersisted(Actor *this, JPH_BodyID /*bodyId*/)
{
	const TriggerMapData *data = this->extraData;
	if (data->enabled)
	{
		TriggerMapChange(data);
	}
}

static void TriggerMapInit(Actor *this, const KvList params, Transform *transform)
{
	this->extraData = malloc(sizeof(TriggerMapData));
	CheckAlloc(this->extraData);
	TriggerMapData *data = this->extraData;
	data->width = KvGetFloat(params, "width", 1.0f);
	data->height = KvGetFloat(params, "height", 1.0f);
	data->depth = KvGetFloat(params, "depth", 1.0f);
	data->enabled = KvGetBool(params, "start_enabled", true);
	data->mapName = strdup(KvGetString(params, "map_name", ""));
	data->entranceName = strdup(KvGetString(params, "entrance_name", ""));
	memcpy(&data->xfm, transform, sizeof(Transform));

	CreateTriggerMapSensor(this, transform);
}

static void TriggerMapDestroy(Actor *this)
{
	const TriggerMapData *data = this->extraData;
	free(data->mapName);
	free(data->entranceName);
}

ActorDefinition triggerMapActorDefinition = {
	.Update = DefaultActorUpdate,
	.OnPlayerContactAdded = DefaultActorOnPlayerContactAdded,
	.OnPlayerContactPersisted = TriggerMapOnPlayerContactPersisted,
	.OnPlayerContactRemoved = DefaultActorOnPlayerContactRemoved,
	.RenderUi = DefaultActorRenderUi,
	.Interact = DefaultActorInteract,
	.Destroy = TriggerMapDestroy,
	.Init = TriggerMapInit,
};

void RegisterTriggerMap()
{
	RegisterDefaultActorInputs(&triggerMapActorDefinition);
	RegisterActorInput(&triggerMapActorDefinition, TRIGGER_MAP_INPUT_FORCE_TRIGGER, TriggerForceTriggerHandler);
	RegisterActorInput(&triggerMapActorDefinition, TRIGGER_MAP_INPUT_ENABLE, TriggerEnableHandler);
	RegisterActorInput(&triggerMapActorDefinition, TRIGGER_MAP_INPUT_DISABLE, TriggerDisableHandler);
	RegisterActor(TRIGGER_MAP_ACTOR_NAME, &triggerMapActorDefinition);
}
