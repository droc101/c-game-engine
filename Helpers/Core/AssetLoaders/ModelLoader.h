//
// Created by droc101 on 7/23/25.
//

#ifndef MODELLOADER_H
#define MODELLOADER_H

#include "../../../defines.h"

/// The maximum number of models that can be loaded in any one execution of the game
#define MAX_MODELS 128

void InitModelLoader();

ModelDefinition *LoadModelInternal(const char* asset);

/**
 * Load a model from an asset
 * @param asset The asset to load the model from
 * @return The loaded model, or NULL if it failed
 */
ModelDefinition *LoadModel(const char *asset);

/**
 * Fetch a cached model from an ID
 * @param id The model ID to fetch
 * @return The model with the given ID
 */
extern ModelDefinition *GetModelFromId(uint id);

void FreeModel(ModelDefinition *model);

void DestroyModelLoader();

#endif //MODELLOADER_H
