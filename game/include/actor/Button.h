//
// Created by droc101 on 3/6/26.
//

#ifndef GAME_WALLBUTTON_H
#define GAME_WALLBUTTON_H

#include <engine/structs/ActorDefinition.h>

extern ActorDefinition buttonActorDefinition;

#define BUTTON_ACTOR_NAME "prop_button"

#define BUTTON_OUTPUT_PRESSED "pressed"
#define BUTTON_OUTPUT_UNPRESSED "unpressed"

void RegisterButton();

#endif //GAME_WALLBUTTON_H
