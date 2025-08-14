//
// Created by droc101 on 5/28/25.
//

#ifndef STATICMODEL_H
#define STATICMODEL_H

#include <joltc.h>
#include "../../Helpers/Core/KVList.h"
#include "../../Structs/Actor.h"

void StaticModelInit(Actor *this, const KvList *params, Transform *transform);

#endif //STATICMODEL_H
