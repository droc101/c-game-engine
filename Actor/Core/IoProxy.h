//
// Created by droc101 on 4/21/25.
//

#ifndef IOPROXY_H
#define IOPROXY_H

#include <box2d/id.h>
#include "../../defines.h"

void IoProxyInit(Actor *this, const b2WorldId worldId, const KvList *params);

void IoProxyUpdate(Actor *this, double /*delta*/);

void IoProxyDestroy(Actor *this);

#endif //IOPROXY_H
