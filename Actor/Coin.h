//
// Created by droc101 on 7/11/2024.
//

#ifndef GAME_COIN_H
#define GAME_COIN_H

#include "../defines.h"

void CoinInit(Actor *this, const KvList *params, JPH_BodyInterface *bodyInterface);

void CoinUpdate(Actor *this, double /*delta*/);

void CoinDestroy(Actor *this);

#endif //GAME_COIN_H
