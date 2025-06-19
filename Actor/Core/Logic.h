//
// Created by droc101 on 6/19/25.
//

#ifndef LOGICBINARY_H
#define LOGICBINARY_H

#include <box2d/id.h>
#include "../../defines.h"

void LogicInit(Actor *this, b2WorldId worldId, const KvList *params);

void LogicDestroy(Actor *this);

#endif //LOGICBINARY_H
