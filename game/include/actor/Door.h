//
// Created by droc101 on 11/7/2024.
//

#ifndef DOOR_H
#define DOOR_H

#include <engine/structs/ActorDefinition.h>

#define ACTOR_TYPE_DOOR (1 + FIRST_GAME_ACTOR_TYPE_ID)

#define DOOR_ACTOR_NAME "prop_door"

#define DOOR_INPUT_OPEN "open"
#define DOOR_INPUT_CLOSE "close"

#define DOOR_OUTPUT_OPENING "opening"
#define DOOR_OUTPUT_CLOSING "closing"
#define DOOR_OUTPUT_FULLY_OPENED "fully_opened"
#define DOOR_OUTPUT_FULLY_CLOSED "fully_closed"

void RegisterDoor();

#endif //DOOR_H
