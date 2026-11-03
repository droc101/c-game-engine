//
// Created by droc101 on 11/3/26.
//

/**
 * @file KvlAsset.h
 */

#ifndef GAME_KVLASSET_H
#define GAME_KVLASSET_H

#include <stdbool.h>
#include <engine/structs/Asset.h>
#include <engine/structs/KVList.h>

/// KvList asset version number
#define KVL_ASSET_VERSION 1

/**
 * Load a KvList from an asset
 * @param output The KvList to load into
 * @param asset The asset to load from
 * @return Success/failure
 */
bool LoadKvlAsset(KvList output, Asset *asset);

#endif //GAME_KVLASSET_H
