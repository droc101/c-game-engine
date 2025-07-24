//
// Created by droc101 on 4/13/25.
//

#ifndef TRIGGER_H
#define TRIGGER_H

#include "../../defines.h"

void TriggerInit(Actor *this, const KvList *params, JPH_BodyInterface *bodyInterface);

void TriggerUpdate(Actor *this, double /*delta*/);

void TriggerDestroy(Actor *this);

#endif //TRIGGER_H
