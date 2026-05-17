//
// Created by droc101 on 4/7/26.
//

#ifndef GAME_CAMERA_ACTOR_H
#define GAME_CAMERA_ACTOR_H

#include <engine/structs/ActorDefinition.h>

extern ActorDefinition cameraActorDefinition;

#define CAMERA_ACTOR_NAME "camera"

#define CAMERA_INPUT_ENABLE "enable"
#define CAMERA_INPUT_DISABLE "disable"

void RegisterCamera();

#endif //GAME_CAMERA_ACTOR_H
