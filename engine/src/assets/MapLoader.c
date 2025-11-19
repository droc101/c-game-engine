//
// Created by droc101 on 4/21/2024.
//

#include <engine/assets/AssetReader.h>
#include <engine/assets/DataReader.h>
#include <engine/assets/MapLoader.h>
#include <engine/assets/MapMaterialLoader.h>
#include <engine/structs/Actor.h>
#include <engine/structs/ActorDefinition.h>
#include <engine/structs/Asset.h>
#include <engine/structs/KVList.h>
#include <engine/structs/List.h>
#include <engine/structs/Map.h>
#include <engine/structs/Param.h>
#include <engine/structs/Vector2.h>
#include <engine/structs/Wall.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Logging.h>
#include <joltc/enums.h>
#include <joltc/joltc.h>
#include <joltc/Math/Quat.h>
#include <joltc/Math/Transform.h>
#include <joltc/Math/Vector3.h>
#include <joltc/Physics/Body/BodyCreationSettings.h>
#include <joltc/Physics/Body/BodyInterface.h>
#include <joltc/Physics/Collision/Shape/Shape.h>
#include <joltc/types.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "engine/assets/ModelLoader.h"
#include "engine/physics/Physics.h"

Map *LoadMap(const char *path)
{
	Map *map = CreateMap();
	Asset *mapData = DecompressAsset(path, false);
	size_t offset = 0;
	JPH_BodyInterface *bodyInterface = JPH_PhysicsSystem_GetBodyInterface(map->physicsSystem);

	map->skyTexture = ReadStringSafe(mapData->data, &offset, mapData->size, NULL);
	map->discordRpcIcon = ReadStringSafe(mapData->data, &offset, mapData->size, NULL);
	map->discordRpcName = ReadStringSafe(mapData->data, &offset, mapData->size, NULL);

	const size_t numActors = ReadSizeT(mapData->data, &offset);
	for (size_t i = 0; i < numActors; i++)
	{
		size_t actorClassLength = 0;
		char *actorClass = ReadStringSafe(mapData->data, &offset, mapData->size, &actorClassLength);
		Transform xfm;
		xfm.position.x = ReadFloat(mapData->data, &offset);
		xfm.position.y = ReadFloat(mapData->data, &offset);
		xfm.position.z = ReadFloat(mapData->data, &offset);
		const float rotX = ReadFloat(mapData->data, &offset);
		const float rotY = ReadFloat(mapData->data, &offset);
		const float rotZ = ReadFloat(mapData->data, &offset);
		Vector3 eulerAngles = {rotX, rotY, rotZ};
		JPH_Quat_FromEulerAngles(&eulerAngles, &xfm.rotation);
		LockingList ioConnections = {0};
		ListInit(ioConnections, LIST_POINTER);
		const size_t numConnections = ReadSizeT(mapData->data, &offset);
		for (size_t j = 0; j < numConnections; j++)
		{
			ActorConnection *conn = malloc(sizeof(ActorConnection));
			conn->sourceActorOutput = ReadStringSafe(mapData->data, &offset, mapData->size, NULL);
			conn->targetActorName = ReadStringSafe(mapData->data, &offset, mapData->size, NULL);
			conn->targetActorInput = ReadStringSafe(mapData->data, &offset, mapData->size, NULL);
			uint8_t hasOverride = ReadByte(mapData->data, &offset);
			ReadParam(mapData->data, mapData->size, &offset, &conn->outParamOverride);
			conn->numRefires = ReadSizeT(mapData->data, &offset);
			ListAdd(ioConnections, conn);
		}
		KvList params;
		KvListCreate(params);
		const size_t numParams = ReadSizeT(mapData->data, &offset);
		for (size_t j = 0; j < numParams; j++)
		{
			char *key = ReadStringSafe(mapData->data, &offset, mapData->size, NULL);
			Param p;
			ReadParam(mapData->data, mapData->size, &offset, &p);
			KvSetUnsafe(params, key, p);
			free(key);
		}

		if (strcmp(actorClass, "player") == 0)
		{
			map->player.transform = xfm;
			KvListDestroy(params);
			ListFree(ioConnections);
			free(actorClass);
			// TODO free stuff
			continue;
		}

		Actor *a = CreateActor(&xfm, actorClass, params, bodyInterface);
		ListFree(a->ioConnections);
		a->ioConnections = ioConnections;
		ListAdd(map->actors, a);
		free(actorClass);
	}

	map->numModels = ReadSizeT(mapData->data, &offset);
	map->models = malloc(sizeof(MapModel) * map->numModels);
	CheckAlloc(map->models);
	for (size_t i = 0; i < map->numModels; i++)
	{
		MapModel *model = &map->models[i];
		char *materialName = ReadStringSafe(mapData->data, &offset, mapData->size, NULL);
		model->material = LoadMapMaterial(materialName);
		free(materialName);

		model->numVerts = ReadUint(mapData->data, &offset);
		model->verts = malloc(sizeof(MapVertex) * model->numVerts);
		for (uint32_t j = 0; j < model->numVerts; j++)
		{
			MapVertex *vtx = &model->verts[j];
			vtx->position.x = ReadFloat(mapData->data, &offset);
			vtx->position.y = ReadFloat(mapData->data, &offset);
			vtx->position.z = ReadFloat(mapData->data, &offset);
			vtx->uv.x = ReadFloat(mapData->data, &offset);
			vtx->uv.y = ReadFloat(mapData->data, &offset);
			vtx->color.r = ReadFloat(mapData->data, &offset);
			vtx->color.g = ReadFloat(mapData->data, &offset);
			vtx->color.b = ReadFloat(mapData->data, &offset);
			vtx->color.a = ReadFloat(mapData->data, &offset);
			vtx->normal.x = ReadFloat(mapData->data, &offset);
			vtx->normal.y = ReadFloat(mapData->data, &offset);
			vtx->normal.z = ReadFloat(mapData->data, &offset);
		}
		model->numIndices = ReadUint(mapData->data, &offset);
		model->indices = malloc(sizeof(uint32_t) * model->numIndices);
		ReadBytes(mapData->data, &offset, sizeof(uint32_t) * model->numIndices, model->indices);
	}


	Transform collisionXfm = {0};
	collisionXfm.rotation = JPH_Quat_Identity;

	const size_t numCollisionMeshes = ReadSizeT(mapData->data, &offset);
	for (size_t i = 0; i < numCollisionMeshes; i++)
	{
		JPH_StaticCompoundShapeSettings *compoundShapeSettings = JPH_StaticCompoundShapeSettings_Create();

		collisionXfm.position.x = ReadFloat(mapData->data, &offset);
		collisionXfm.position.y = ReadFloat(mapData->data, &offset);
		collisionXfm.position.z = ReadFloat(mapData->data, &offset);
		const size_t numSubShapes = ReadSizeT(mapData->data, &offset);
		for (size_t j = 0; j < numSubShapes; j++)
		{
			ModelStaticCollider staticCollider;
			staticCollider.numTriangles = ReadSizeT(mapData->data, &offset);
			staticCollider.tris = malloc(sizeof(JPH_Triangle) * staticCollider.numTriangles);
			CheckAlloc(staticCollider.tris);
			for (size_t k = 0; k < staticCollider.numTriangles; k++)
			{
				JPH_Triangle *triangle = &staticCollider.tris[k];
				triangle->materialIndex = 0;
				Vector3 *verts[3] = {&triangle->v1, &triangle->v2, &triangle->v3};
				for (int l = 0; l < 3; l++)
				{
					Vector3 *point = verts[l];
					point->x = ReadFloat(mapData->data, &offset);
					point->y = ReadFloat(mapData->data, &offset);
					point->z = ReadFloat(mapData->data, &offset);
				}
			}
			const JPH_Shape *shape = CreateStaticModelShape(&staticCollider);

			JPH_CompoundShapeSettings_AddShape2((JPH_CompoundShapeSettings *)compoundShapeSettings,
												&Vector3_Zero,
												&JPH_Quat_Identity,
												shape,
												0);

			JPH_Shape_Destroy(shape);
			free(staticCollider.tris);
		}
		JPH_Shape *shape = (JPH_Shape *)JPH_StaticCompoundShape_Create(compoundShapeSettings);

		JPH_BodyCreationSettings *bodyCreationSettings = JPH_BodyCreationSettings_Create2_GAME(shape,
																							   &collisionXfm,
																							   JPH_MotionType_Static,
																							   OBJECT_LAYER_STATIC,
																							   0);
		JPH_BodyId body = JPH_BodyInterface_CreateAndAddBody(
				bodyInterface,
				bodyCreationSettings,
				JPH_Activation_Activate); // TODO this will likely need to be freed
		JPH_BodyCreationSettings_Destroy(bodyCreationSettings);
	}

	JPH_PhysicsSystem_OptimizeBroadPhase(map->physicsSystem);

	FreeAsset(mapData);

	return map;
}
