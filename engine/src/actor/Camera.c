//
// Created by droc101 on 4/7/26.
//

#include <engine/actor/Camera.h>
#include <engine/graphics/RenderingHelpers.h>
#include <engine/helpers/PlatformHelpers.h>
#include <engine/structs/Actor.h>
#include <engine/structs/ActorDefinition.h>
#include <engine/structs/Camera.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/KVList.h>
#include <engine/subsystem/Error.h>
#include <joltc/Math/Transform.h>
#include <joltc/Physics/Body/BodyInterface.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

typedef struct CameraData
{
	Camera camera;
} CameraData;

static void CameraInit(Actor *this, const KvList params, Transform *transform)
{
	this->extraData = AvxAlignedCalloc(sizeof(CameraData));
	CheckAlloc(this->extraData);
	CameraData *data = this->extraData;
	memcpy(&data->camera.transform, transform, sizeof(Transform));
	data->camera.fov = (float)KvGetInt(params, "fov", 90);
	data->camera.nearPlane = KvGetFloat(params, "near_plane", DEFAULT_NEAR_PLANE);
	data->camera.farPlane = KvGetFloat(params, "far_plane", DEFAULT_FAR_PLANE);
	data->camera.showPlayerModel = KvGetBool(params, "show_player_model", true);
	data->camera.recomputeCachedData = true;
	ActorCreateEmptyBody(this, transform);
}

static void CameraUpdate(Actor *this, double /*delta*/)
{
	CameraData *data = this->extraData;
	if (GetState()->camera == &data->camera)
	{
		JPH_BodyInterface_GetPositionAndRotation(this->bodyInterface,
												 this->bodyId,
												 &data->camera.transform.position,
												 &data->camera.transform.rotation);
	}
}

static void CameraDestroy(Actor *this)
{
	AvxAlignedFree(this->extraData);
	this->extraData = NULL;
}

static void CameraEnableHandler(Actor *this, const Actor * /*sender*/, const Param * /*param*/)
{
	CameraData *data = this->extraData;
	GetState()->camera = &data->camera;
}

static void CameraDisableHandler(Actor *this, const Actor * /*sender*/, const Param * /*param*/)
{
	const CameraData *data = this->extraData;
	if (GetState()->camera == &data->camera)
	{
		GetState()->camera = &GetState()->map->player.playerCamera;
	}
}

ActorDefinition cameraActorDefinition = {
	.Update = CameraUpdate,
	.OnPlayerContactAdded = DefaultActorOnPlayerContactAdded,
	.OnPlayerContactPersisted = DefaultActorOnPlayerContactPersisted,
	.OnPlayerContactRemoved = DefaultActorOnPlayerContactRemoved,
	.RenderUi = DefaultActorRenderUi,
	.Interact = DefaultActorInteract,
	.Destroy = CameraDestroy,
	.Init = CameraInit,
};

void RegisterCamera()
{
	RegisterDefaultActorInputs(&cameraActorDefinition);
	RegisterActorInput(&cameraActorDefinition, CAMERA_INPUT_ENABLE, CameraEnableHandler);
	RegisterActorInput(&cameraActorDefinition, CAMERA_INPUT_DISABLE, CameraDisableHandler);
	RegisterActor(CAMERA_ACTOR_NAME, &cameraActorDefinition);
}
