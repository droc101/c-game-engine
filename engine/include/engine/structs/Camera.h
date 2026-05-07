//
// Created by droc101 on 8/13/25.
//

#ifndef GAME_CAMERA_H
#define GAME_CAMERA_H

#include <joltc/Math/Transform.h>
#include <stdbool.h>

typedef struct Camera Camera;

struct Camera
{
	/// The 3d transform of the camera
	Transform transform;
	/// The field of view of the camera
	float fov;
	/// Whether the 3rd person player model should be rendered
	bool showPlayerModel;
};

#endif //GAME_CAMERA_H
