//
// Created by droc101 on 4/21/2024.
//

#include <engine/assets/AssetReader.h>
#include <engine/assets/DataReader.h>
#include <engine/assets/MapLoader.h>
#include <engine/assets/MapMaterialLoader.h>
#include <engine/assets/ModelLoader.h>
#include <engine/physics/Physics.h>
#include <engine/structs/Actor.h>
#include <engine/structs/ActorDefinition.h>
#include <engine/structs/ActorWall.h>
#include <engine/structs/Asset.h>
#include <engine/structs/KVList.h>
#include <engine/structs/List.h>
#include <engine/structs/Map.h>
#include <engine/structs/Param.h>
#include <engine/structs/Vector2.h>
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

Map *LoadMap(const char *path)
{
	Map *map = CreateMap();
	Asset *mapData = DecompressAsset(path, false);
	size_t offset = 0;
	size_t bytesRemaining = mapData->size;
	size_t strLength = 0;

	JPH_BodyInterface *bodyInterface = JPH_PhysicsSystem_GetBodyInterface(map->physicsSystem);

	map->skyTexture = ReadStringSafe(mapData->data, &offset, mapData->size, &strLength);
	bytesRemaining -= strLength;
	bytesRemaining += sizeof(size_t);
	map->discordRpcIcon = ReadStringSafe(mapData->data, &offset, mapData->size, &strLength);
	bytesRemaining -= strLength;
	bytesRemaining += sizeof(size_t);
	map->discordRpcName = ReadStringSafe(mapData->data, &offset, mapData->size, &strLength);
	bytesRemaining -= strLength;
	bytesRemaining += sizeof(size_t);

	EXPECT_BYTES(sizeof(size_t), bytesRemaining);
	const size_t numActors = ReadSizeT(mapData->data, &offset);
	for (size_t i = 0; i < numActors; i++)
	{
		size_t actorClassLength = 0;
		char *actorClass = ReadStringSafe(mapData->data, &offset, mapData->size, &actorClassLength);
		bytesRemaining -= actorClassLength;
		bytesRemaining -= sizeof(size_t);

		EXPECT_BYTES(sizeof(float) * 6, bytesRemaining);
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
		EXPECT_BYTES(sizeof(size_t), bytesRemaining);
		const size_t numConnections = ReadSizeT(mapData->data, &offset);
		for (size_t j = 0; j < numConnections; j++)
		{
			ActorConnection *connection = malloc(sizeof(ActorConnection));
			CheckAlloc(connection);
			connection->sourceActorOutput = ReadStringSafe(mapData->data, &offset, mapData->size, &strLength);
			bytesRemaining -= strLength;
			bytesRemaining += sizeof(size_t);
			connection->targetActorName = ReadStringSafe(mapData->data, &offset, mapData->size, &strLength);
			bytesRemaining -= strLength;
			bytesRemaining += sizeof(size_t);
			connection->targetActorInput = ReadStringSafe(mapData->data, &offset, mapData->size, &strLength);
			bytesRemaining -= strLength;
			bytesRemaining += sizeof(size_t);
			uint8_t hasOverride = ReadByte(mapData->data, &offset);
			// TODO data size validation for params
			size_t paramSize = ReadParam(mapData->data, mapData->size, &offset, &connection->outParamOverride);
			bytesRemaining -= paramSize;
			connection->numRefires = ReadSizeT(mapData->data, &offset);
			ListAdd(ioConnections, connection);
		}
		KvList params;
		KvListCreate(params);
		EXPECT_BYTES(sizeof(size_t), bytesRemaining);
		const size_t numParams = ReadSizeT(mapData->data, &offset);
		for (size_t j = 0; j < numParams; j++)
		{
			char *key = ReadStringSafe(mapData->data, &offset, mapData->size, &strLength);
			bytesRemaining -= sizeof(size_t);
			bytesRemaining -= strLength;
			Param param;
			bytesRemaining -= ReadParam(mapData->data, mapData->size, &offset, &param);
			KvSetUnsafe(params, key, param);
			free(key);
		}

		if (strcmp(actorClass, "player") == 0)
		{
			map->player.transform = xfm;
			KvListDestroy(params);
			ListFree(ioConnections);
			free(actorClass);
			continue;
		}

		const char *actorName = NULL;
		if (KvHas(params, "name", PARAM_TYPE_STRING))
		{
			actorName = KvGetString(params, "name", "");
			if (actorName[0] != '\0')
			{
				actorName = strdup(actorName);
			} else
			{
				actorName = NULL;
			}
		}

		Actor *actor = CreateActor(&xfm, actorClass, params, bodyInterface);
		ListFree(actor->ioConnections);
		actor->ioConnections = ioConnections;
		ListAdd(map->actors, actor);
		free(actorClass);

		if (actorName && actorName[0] != '\0')
		{
			ListAdd(map->namedActorNames, actorName);
			ListAdd(map->namedActorPointers, actor);
		}
	}

	EXPECT_BYTES(sizeof(size_t), bytesRemaining);
	map->modelCount = ReadSizeT(mapData->data, &offset);
	map->models = malloc(sizeof(MapModel) * map->modelCount);
	CheckAlloc(map->models);
	for (size_t i = 0; i < map->modelCount; i++)
	{
		MapModel *model = &map->models[i];
		char *materialName = ReadStringSafe(mapData->data, &offset, mapData->size, &strLength);
		bytesRemaining -= sizeof(size_t);
		bytesRemaining -= strLength;
		model->material = LoadMapMaterial(materialName);
		free(materialName);

		EXPECT_BYTES(sizeof(uint32_t), bytesRemaining);
		model->vertexCount = ReadUint(mapData->data, &offset);
		model->vertices = malloc(sizeof(MapVertex) * model->vertexCount);
		CheckAlloc(model->vertices);
		for (uint32_t j = 0; j < model->vertexCount; j++)
		{
			MapVertex *vertex = model->vertices + j;
			EXPECT_BYTES(sizeof(float) * 12, bytesRemaining);
			vertex->position.x = ReadFloat(mapData->data, &offset);
			vertex->position.y = ReadFloat(mapData->data, &offset);
			vertex->position.z = ReadFloat(mapData->data, &offset);
			vertex->uv.x = ReadFloat(mapData->data, &offset);
			vertex->uv.y = ReadFloat(mapData->data, &offset);
			vertex->color.r = ReadFloat(mapData->data, &offset);
			vertex->color.g = ReadFloat(mapData->data, &offset);
			vertex->color.b = ReadFloat(mapData->data, &offset);
			vertex->color.a = ReadFloat(mapData->data, &offset);
			vertex->normal.x = ReadFloat(mapData->data, &offset);
			vertex->normal.y = ReadFloat(mapData->data, &offset);
			vertex->normal.z = ReadFloat(mapData->data, &offset);
		}
		EXPECT_BYTES(sizeof(uint32_t), bytesRemaining);
		model->indexCount = ReadUint(mapData->data, &offset);
		EXPECT_BYTES(sizeof(uint32_t) * model->indexCount, bytesRemaining);
		model->indices = malloc(sizeof(uint32_t) * model->indexCount);
		CheckAlloc(model->indices);
		ReadBytes(mapData->data, &offset, sizeof(uint32_t) * model->indexCount, model->indices);
	}


	Transform collisionXfm = {
		.rotation = JPH_Quat_Identity,
	};

	EXPECT_BYTES(sizeof(size_t), bytesRemaining);
	const size_t numCollisionMeshes = ReadSizeT(mapData->data, &offset);
	for (size_t i = 0; i < numCollisionMeshes; i++)
	{
		EXPECT_BYTES((sizeof(float) * 3) + sizeof(size_t), bytesRemaining);
		collisionXfm.position.x = ReadFloat(mapData->data, &offset);
		collisionXfm.position.y = ReadFloat(mapData->data, &offset);
		collisionXfm.position.z = ReadFloat(mapData->data, &offset);
		const size_t subShapeCount = ReadSizeT(mapData->data, &offset);
		if (subShapeCount == 0)
		{
			continue;
		}

		JPH_StaticCompoundShapeSettings *compoundShapeSettings = JPH_StaticCompoundShapeSettings_Create();

		for (size_t j = 0; j < subShapeCount; j++)
		{
			ModelStaticCollider staticCollider;
			EXPECT_BYTES(sizeof(size_t), bytesRemaining);
			staticCollider.numTriangles = ReadSizeT(mapData->data, &offset);
			staticCollider.tris = malloc(sizeof(JPH_Triangle) * staticCollider.numTriangles);
			CheckAlloc(staticCollider.tris);
			for (size_t k = 0; k < staticCollider.numTriangles; k++)
			{
				JPH_Triangle *triangle = &staticCollider.tris[k];
				triangle->materialIndex = 0;
				EXPECT_BYTES(sizeof(float) * 9, bytesRemaining);

				triangle->v1.x = ReadFloat(mapData->data, &offset);
				triangle->v1.y = ReadFloat(mapData->data, &offset);
				triangle->v1.z = ReadFloat(mapData->data, &offset);

				triangle->v2.x = ReadFloat(mapData->data, &offset);
				triangle->v2.y = ReadFloat(mapData->data, &offset);
				triangle->v2.z = ReadFloat(mapData->data, &offset);

				triangle->v3.x = ReadFloat(mapData->data, &offset);
				triangle->v3.y = ReadFloat(mapData->data, &offset);
				triangle->v3.z = ReadFloat(mapData->data, &offset);
			}
			JPH_Shape *subShape = CreateStaticModelShape(&staticCollider);

			JPH_CompoundShapeSettings_AddShape2((JPH_CompoundShapeSettings *)compoundShapeSettings,
												&Vector3_Zero,
												&JPH_Quat_Identity,
												subShape,
												0);

			JPH_Shape_Destroy(subShape);
			free(staticCollider.tris);
		}
		JPH_Shape *shape = (JPH_Shape *)JPH_StaticCompoundShape_Create(compoundShapeSettings);

		JPH_BodyCreationSettings *bodyCreationSettings = JPH_BodyCreationSettings_Create2_GAME(shape,
																							   &collisionXfm,
																							   JPH_MotionType_Static,
																							   OBJECT_LAYER_STATIC,
																							   0);
		JPH_BodyCreationSettings_SetFriction(bodyCreationSettings, 4.25f);
		const JPH_BodyId body = JPH_BodyInterface_CreateAndAddBody(bodyInterface,
																   bodyCreationSettings,
																   JPH_Activation_Activate);
		ListAdd(map->joltBodies, body);
		JPH_BodyCreationSettings_Destroy(bodyCreationSettings);
		JPH_ShapeSettings_Destroy((JPH_ShapeSettings *)compoundShapeSettings);
		JPH_Shape_Destroy(shape);
	}

	JPH_PhysicsSystem_OptimizeBroadPhase(map->physicsSystem);

	FreeAsset(mapData);

	return map;
}
