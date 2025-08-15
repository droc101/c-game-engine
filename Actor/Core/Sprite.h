//
// Created by droc101 on 5/29/25.
//

#ifndef SPRITE_H
#define SPRITE_H

#include <joltc/Math/Transform.h>
#include "../../Helpers/Core/KVList.h"
#include "../../Structs/Actor.h"

void SpriteInit(Actor *this, const KvList *params, Transform *transform);

#endif //SPRITE_H
