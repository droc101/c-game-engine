//
// Created by droc101 on 5/29/25.
//

#ifndef SOUNDPLAYER_H
#define SOUNDPLAYER_H

#include <box2d/id.h>
#include "../../defines.h"

void SoundPlayerInit(Actor *this, b2WorldId worldId, KvList *params);

void SoundPlayerDestroy(Actor *this);

#endif //SOUNDPLAYER_H
