//
// Created by droc101 on 7/11/2024.
//

#ifndef GAME_COIN_H
#define GAME_COIN_H

#include "../defines.h"

void CoinInit(Actor *this, const KvList *params, Transform *transform);

void CoinUpdate(Actor *this, double /*delta*/);

#endif //GAME_COIN_H
