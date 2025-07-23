//
// Created by droc101 on 6/19/25.
//

#ifndef LOGICBINARY_H
#define LOGICBINARY_H

#include <box2d/id.h>
#include "../../defines.h"

void LogicBinaryInit(Actor *this, b2WorldId worldId, const KvList *params);

void LogicBinaryDestroy(Actor *this);

#endif //LOGICBINARY_H
