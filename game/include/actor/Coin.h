//
// Created by droc101 on 7/11/2024.
//

#ifndef GAME_COIN_H
#define GAME_COIN_H

#include <engine/structs/ActorDefinition.h>

extern ActorDefinition coinActorDefinition;

#define COIN_ACTOR_NAME "prop_coin"

#define COIN_OUTPUT_COLLECTED "collected"

void RegisterCoin();

#endif //GAME_COIN_H
