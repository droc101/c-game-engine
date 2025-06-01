//
// Created by droc101 on 5/29/25.
//

#ifndef SPRITE_H
#define SPRITE_H

#include <box2d/id.h>
#include "../../defines.h"

void SpriteInit(Actor *this, b2WorldId, const KvList *params);

void SpriteDestroy(Actor *this);

#endif //SPRITE_H
