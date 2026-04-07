//
// Created by droc101 on 4/7/26.
//

#include <engine/actor/Camera.h>
#include <engine/structs/Camera.h>
#include "engine/structs/Actor.h"
#include "engine/structs/GlobalState.h"

typedef struct CameraData
{
	Camera camera;
} CameraData;

void CameraInit(Actor *this, const KvList params, Transform *transform)
{
	this->extraData = calloc(1, sizeof(CameraData));
	CheckAlloc(this->extraData);
	CameraData *data = this->extraData;
	memcpy(&data->camera.transform, transform, sizeof(Transform));
	data->camera.fov = KvGetFloat(params, "fov", 90.0f);
	data->camera.showPlayerModel = KvGetBool(params, "show_player_model", true);
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
	.Destroy = DefaultActorDestroy,
	.Init = CameraInit,
};

void RegisterCamera()
{
	RegisterDefaultActorInputs(&cameraActorDefinition);
	RegisterActorInput(&cameraActorDefinition, CAMERA_INPUT_ENABLE, CameraEnableHandler);
	RegisterActorInput(&cameraActorDefinition, CAMERA_INPUT_DISABLE, CameraDisableHandler);
	RegisterActor(CAMERA_ACTOR_NAME, &cameraActorDefinition);
}
