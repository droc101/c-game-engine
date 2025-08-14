//
// Created by droc101 on 4/13/25.
//

#ifndef TRIGGER_H
#define TRIGGER_H

#include <joltc.h>
#include "../../Helpers/Core/KVList.h"
#include "../../Structs/Actor.h"

void TriggerInit(Actor *this, const KvList *params, Transform *transform);

#endif //TRIGGER_H
