//
// Created by droc101 on 3/20/26.
//

#ifndef GAME_TONEMAPCONTROLLER_H
#define GAME_TONEMAPCONTROLLER_H

#include <engine/structs/ActorDefinition.h>

extern ActorDefinition tonemapControllerActorDefinition;

#define TONEMAP_CONTROLLER_ACTOR_NAME "tonemap_controller"

#define TONEMAP_CONTROLLER_INPUT_SET "set"
#define TONEMAP_CONTROLLER_INPUT_SET_INSTANT "set_instant"

void RegisterTonemapController();

#endif //GAME_TONEMAPCONTROLLER_H
