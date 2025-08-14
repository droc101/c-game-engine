//
// Created by droc101 on 8/13/25.
//

#ifndef GAME_CAMERA_H
#define GAME_CAMERA_H

#include <joltc.h>

typedef struct Camera Camera;

struct Camera
{
	/// The 3d transform of the camera
	Transform transform;
	/// The y-offset, used for view bobbing
	float yOffset;

	/// The field of view of the camera
	float fov;
};

#endif //GAME_CAMERA_H
