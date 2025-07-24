//
// Created by droc101 on 4/21/25.
//

#ifndef IOPROXY_H
#define IOPROXY_H

#include "../../defines.h"

void IoProxyInit(Actor *this, const KvList *params, JPH_BodyInterface *bodyInterface);

void IoProxyUpdate(Actor *this, double /*delta*/);

void IoProxyDestroy(Actor *this);

#endif //IOPROXY_H
