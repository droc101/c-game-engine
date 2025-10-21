//
// Created by droc101 on 8/13/25.
//

#ifndef GAME_VIEWMODEL_H
#define GAME_VIEWMODEL_H

#include <joltc/Math/Transform.h>
#include <stdbool.h>
#include <stdint.h>
#include <engine/assets/ModelLoader.h>

typedef struct Viewmodel Viewmodel;

struct Viewmodel
{
	/// Whether to show the viewmodel
	bool enabled;
	/// The model to render
	ModelDefinition *model;
	/// The skin to render
	uint32_t modelSkin;
	/// The transform to render
	Transform transform;
};

#endif //GAME_VIEWMODEL_H
