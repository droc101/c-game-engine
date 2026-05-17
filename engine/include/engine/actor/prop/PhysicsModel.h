//
// Created by droc101 on 3/2/26.
//

#ifndef GAME_PHYSICSMODEL_H
#define GAME_PHYSICSMODEL_H

#include <engine/structs/ActorDefinition.h>

extern ActorDefinition physicsModelActorDefinition;

#define PHYSICS_MODEL_ACTOR_NAME "prop_model_physics"

#define PHYSICS_MODEL_INPUT_SET_SKIN "set_skin"
#define PHYSICS_MODEL_INPUT_SET_COLOR "set_color"

void RegisterPhysicsModel();

#endif //GAME_PHYSICSMODEL_H
