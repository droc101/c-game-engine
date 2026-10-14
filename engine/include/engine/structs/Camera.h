//
// Created by droc101 on 8/13/25.
//

#ifndef GAME_CAMERA_H
#define GAME_CAMERA_H

#include <cglm/cglm.h>
#include <joltc/Math/Transform.h>
#include <stdbool.h>

typedef struct Camera Camera;

struct Camera
{
	/// The 3d transform of the camera
	Transform transform;
	/// The field of view of the camera
	float fov;
	/// Near plane distance
	float nearPlane;
	/// Far plane distance
	float farPlane;
	/// Whether the 3rd person player model should be rendered
	bool showPlayerModel;
	/// Set to true if cached projection/frustum data needs to be recomputed
	bool recomputeCachedData;
	/// Cached frustum planes
	float frustumPlanes[4];
	/// Cached projection matrix
	CGLM_ALIGN_MAT mat4 projectionMatrix;
};

#endif //GAME_CAMERA_H
