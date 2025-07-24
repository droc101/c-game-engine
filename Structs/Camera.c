//
// Created by droc101 on 10/2/24.
//

#include "Camera.h"
#include <math.h>
#include "../Helpers/Core/Error.h"

/**
 * Create a camera with default values
 * @return A pointer to the camera
 */
Camera *CreateCamera()
{
	Camera *camera = calloc(1, sizeof(Camera));
	CheckAlloc(camera);
	camera->fov = FOV;
	return camera;
}

/**
 * Look at a target. The XY of the target vector is XZ in 3d, with Y=0
 * @param camera The camera to look with
 * @param target The target to look at
 */
void CameraLookAt(Camera *camera, const Vector2 target)
{
	camera->yaw = atan2f(target.x - camera->x, target.y - camera->z);
	camera->pitch = 0;
}
