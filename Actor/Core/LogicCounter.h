//
// Created by droc101 on 7/22/25.
//

#ifndef LOGICCOUNTER_H
#define LOGICCOUNTER_H

#include <box2d/id.h>
#include "../../defines.h"

void LogicCounterInit(Actor *this, b2WorldId worldId, const KvList *params);

void LogicCounterDestroy(Actor *this);

#endif //LOGICCOUNTER_H
