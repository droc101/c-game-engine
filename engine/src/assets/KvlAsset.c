//
// Created by droc101 on 11/3/26.
//

#include <engine/assets/DataReader.h>
#include <engine/assets/KvlAsset.h>
#include <engine/structs/Asset.h>
#include <engine/structs/KVList.h>
#include <stdbool.h>
#include <stddef.h>

bool LoadKvlAsset(KvList output, Asset *asset)
{
	if (!asset || asset->type != ASSET_TYPE_KV_LIST || asset->typeVersion != KVL_ASSET_VERSION)
	{
		return false;
	}
	DataReader *reader = CreateDataReaderFromAsset(asset);
	size_t bytesRemaining = asset->size;
	if (!ReadKvList(reader, output, &bytesRemaining))
	{
		return false;
	}
	DestroyDataReader(reader);
	return true;
}
