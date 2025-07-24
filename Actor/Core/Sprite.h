//
// Created by droc101 on 5/29/25.
//

#ifndef SPRITE_H
#define SPRITE_H

#include "../../defines.h"

void SpriteInit(Actor *this, const KvList *params, JPH_BodyInterface *bodyInterface);

void SpriteDestroy(Actor *this);

#endif //SPRITE_H
