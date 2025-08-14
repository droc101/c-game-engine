//
// Created by droc101 on 7/11/2024.
//

#ifndef GAME_COIN_H
#define GAME_COIN_H

#include <joltc.h>
#include "../Helpers/Core/KVList.h"
#include "../Structs/Actor.h"

void CoinInit(Actor *this, const KvList *params, Transform *transform);

#endif //GAME_COIN_H
