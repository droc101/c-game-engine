//
// Created by droc101 on 5/29/25.
//

#ifndef SOUND_PLAYER_H
#define SOUND_PLAYER_H

#include <joltc/Math/Transform.h>
#include "../../Helpers/Core/KVList.h"
#include "../../Structs/Actor.h"

void SoundPlayerInit(Actor *this, const KvList *params, Transform *transform);

#endif //SOUND_PLAYER_H
