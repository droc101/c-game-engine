//
// Created by droc101 on 7/22/25.
//

#ifndef LOGICDECIMAL_H
#define LOGICDECIMAL_H

#include <box2d/id.h>
#include "../../defines.h"

void LogicDecimalInit(Actor *this, b2WorldId worldId, const KvList *params);

void LogicDecimalDestroy(Actor *this);

#endif //LOGICDECIMAL_H
