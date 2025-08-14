//
// Created by droc101 on 4/21/25.
//

#ifndef IOPROXY_H
#define IOPROXY_H

#include <joltc.h>
#include "../../Helpers/Core/KVList.h"
#include "../../Structs/Actor.h"

void IoProxyInit(Actor *this, const KvList *params, Transform *transform);

#endif //IOPROXY_H
