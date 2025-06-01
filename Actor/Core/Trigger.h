//
// Created by droc101 on 4/13/25.
//

#ifndef TRIGGER_H
#define TRIGGER_H
#include <box2d/id.h>
#include "../../defines.h"

void TriggerInit(Actor *this, b2WorldId worldId, const KvList *params);

void TriggerUpdate(Actor *this, double /*delta*/);

void TriggerDestroy(Actor *this);

#endif //TRIGGER_H
