//
// Created by droc101 on 7/7/2024.
//

#ifndef GAME_COMMONASSETS_H
#define GAME_COMMONASSETS_H

#include "Core/AssetLoaders/FontLoader.h"

extern Font *smallFont;
extern Font *largeFont;

/**
 * Initialize common assets
 */
void InitCommonAssets();

/**
 * Free any common assets that were allocated
 */
void DestroyCommonAssets();

#endif //GAME_COMMONASSETS_H
