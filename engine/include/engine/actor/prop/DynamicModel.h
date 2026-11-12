//
// Created by droc101 on 11/7/26.
//

#ifndef GAME_DYNAMICMODEL_H
#define GAME_DYNAMICMODEL_H

#include <engine/structs/ActorDefinition.h>

extern ActorDefinition dynamicModelActorDefinition;

#define DYNAMIC_MODEL_ACTOR_NAME "prop_model_dynamic"

#define DYNAMIC_MODEL_INPUT_SET_SKIN "set_skin"
#define DYNAMIC_MODEL_INPUT_SET_COLOR "set_color"

void RegisterDynamicModel();

#endif //GAME_DYNAMICMODEL_H
