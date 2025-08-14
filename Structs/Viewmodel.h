//
// Created by droc101 on 8/13/25.
//

#ifndef GAME_VIEWMODEL_H
#define GAME_VIEWMODEL_H

#include <joltc.h>
#include <stdbool.h>
#include <stdint.h>
#include "../Helpers/Core/AssetLoaders/ModelLoader.h"

typedef struct Viewmodel Viewmodel;

struct Viewmodel
{
	bool enabled;
	ModelDefinition *model;
	uint32_t modelSkin;
	Transform transform;
};

#endif //GAME_VIEWMODEL_H
