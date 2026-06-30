//
// Created by droc101 on 6/28/26.
//

#include <engine/actor/Entrance.h>
#include <engine/physics/PlayerPhysics.h>
#include <engine/structs/Actor.h>
#include <engine/structs/ActorDefinition.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/KVList.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Logging.h>
#include <joltc/Math/Quat.h>
#include <joltc/Math/RVec3.h>
#include <joltc/Math/Transform.h>
#include <joltc/Math/Vector3.h>
#include <SDL3/SDL_stdinc.h>
#include <stdlib.h>
#include <string.h>

typedef struct EntranceData
{
	char *entranceName;
	Transform xfm;
} EntranceData;

static void EntranceUpdate(Actor *this, double /*delta*/)
{
	if (GetState()->map->physicsTick == 0)
	{
		const EntranceData *data = this->extraData;
		if (GetState()->map->transition && strcmp(data->entranceName, GetState()->map->transition->entranceName) == 0)
		{
			const MapTransition *transition = GetState()->map->transition;
			LogDebug("Entrance \"%s\" activated\n", data->entranceName);

			Vector3 thisRot;
			JPH_Quat_GetEulerAngles(&data->xfm.rotation, &thisRot);

			Vector3 absoluteRot;
			JPH_RVec3_Add(&thisRot, &transition->relativeAngles, &absoluteRot);

			Transform finalXfm;
			JPH_Quat_FromEulerAngles(&absoluteRot, &finalXfm.rotation);

			JPH_RVec3_Add(&data->xfm.position, &transition->relativePosition, &finalXfm.position);

			SetPlayerTransform(&GetState()->map->player, &finalXfm);
			ActorFireOutput(this, ENTRANCE_OUTPUT_USED, PARAM_NONE);
		}
	}
}

static void EntranceInit(Actor *this, const KvList params, Transform *transform)
{
	ActorCreateEmptyBody(this, transform);
	this->extraData = malloc(sizeof(EntranceData));
	CheckAlloc(this->extraData);
	EntranceData *data = this->extraData;
	data->entranceName = strdup(KvGetString(params, "name", ""));
	memcpy(&data->xfm, transform, sizeof(Transform));
}

ActorDefinition entranceActorDefinition = {
	.Update = EntranceUpdate,
	.OnPlayerContactAdded = DefaultActorOnPlayerContactAdded,
	.OnPlayerContactPersisted = DefaultActorOnPlayerContactPersisted,
	.OnPlayerContactRemoved = DefaultActorOnPlayerContactRemoved,
	.RenderUi = DefaultActorRenderUi,
	.Interact = DefaultActorInteract,
	.Destroy = DefaultActorDestroy,
	.Init = EntranceInit,
};

void RegisterEntrance()
{
	RegisterDefaultActorInputs(&entranceActorDefinition);
	RegisterActor(ENTRANCE_ACTOR_NAME, &entranceActorDefinition);
}
