//
// Created by NBT22 on 10/20/26.
//

#ifndef GAME_VULKANMODEL_H
#define GAME_VULKANMODEL_H

#include <engine/assets/ModelLoader.h>
#include <engine/structs/List.h>
#include <stddef.h>
#include <stdint.h>

typedef struct
{
	uint32_t indexCount;
	uint32_t firstIndex;
	int32_t vertexOffset;
} MaterialSlotVertexData;

/// A list of uint32_t model ids that are currently loaded
extern List loadedModelIds;
/// A list, indexed with a lod id, that contains lists of @c MaterialSlotVertexData structures for each material slot
extern List lodMaterialSlotsVertexData;

/// The number of currently allocated buffers. Should match @c frustumCount
extern uint32_t allocatedBufferCount;
/// The number of loaded vertices
extern size_t bufferVertexCount;
/// The number of loaded indices
extern size_t bufferIndexCount;

void LoadModelLods(const ModelDefinition *model);

void ClearModelCache();

#endif //GAME_VULKANMODEL_H
