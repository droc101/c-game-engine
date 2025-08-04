//
// Created by droc101 on 5/28/25.
//

#include "StaticModel.h"
#include "../../Helpers/Core/AssetReader.h"
#include "../../Helpers/Core/KVList.h"


void StaticModelInit(Actor *this, const KvList *params)
{
	char modelPath[80];
	snprintf(modelPath, 80, "model/%s.gmdl", KvGetString(params, "model", "model_leafy"));
	this->actorModel = LoadModel(modelPath);
	this->transform.position.y = KvGetFloat(params, "yPosition", 0.0f);
	this->currentSkinIndex = KvGetInt(params, "skin", 0);

	const Vector3 points[] = {
		{-0.250000f, 0.250000f, 0.250000f},
		{-0.250000f, -0.250000f, 0.250000f},
		{-0.250000f, 0.250000f, -0.250000f},
		{-0.250000f, -0.250000f, -0.250000f},
		{0.250000f, 0.250000f, 0.250000f},
		{0.250000f, -0.250000f, 0.250000f},
		{0.250000f, 0.250000f, -0.250000f},
		{0.250000f, -0.250000f, -0.250000f},
		{-0.000000f, 0.250000f, -0.500000f},
		{-0.000000f, -0.250000f, -0.500000f},
		{-0.000000f, 0.250000f, -0.500000f},
		{-0.000000f, -0.250000f, -0.500000f},
	};
	const JPH_ConvexHullShapeSettings *shapeSettings = JPH_ConvexHullShapeSettings_Create(points,
																						  sizeof(points) /
																								  sizeof(*points),
																						  JPH_DEFAULT_CONVEX_RADIUS);
	JPH_Quat rotation = {};
	JPH_Quat_FromEulerAngles(&this->transform.rotation, &rotation);
	JPH_BodyCreationSettings
			*settings = JPH_BodyCreationSettings_Create3((const JPH_Shape *)
																 JPH_ConvexHullShapeSettings_CreateShape(shapeSettings),
														 &this->transform.position,
														 &rotation,
														 JPH_MotionType_Static,
														 OBJECT_LAYER_STATIC);
	JPH_BodyCreationSettings_SetIsSensor(settings, true);
	this->bodyId = JPH_BodyInterface_CreateAndAddBody(this->bodyInterface, settings, JPH_Activation_DontActivate);
	JPH_BodyCreationSettings_Destroy(settings);
}
