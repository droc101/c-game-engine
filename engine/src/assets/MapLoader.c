//
// Created by droc101 on 4/21/2024.
//

#include <assert.h>
#include <engine/assets/AssetReader.h>
#include <engine/assets/DataReader.h>
#include <engine/assets/MapLoader.h>
#include <engine/assets/MapMaterialLoader.h>
#include <engine/assets/ModelLoader.h>
#include <engine/graphics/RenderingHelpers.h>
#include <engine/physics/Physics.h>
#include <engine/physics/PlayerPhysics.h>
#include <engine/structs/Actor.h>
#include <engine/structs/ActorDefinition.h>
#include <engine/structs/ActorWall.h>
#include <engine/structs/Asset.h>
#include <engine/structs/KVList.h>
#include <engine/structs/Light.h>
#include <engine/structs/List.h>
#include <engine/structs/Map.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Error.h>
#include <joltc/enums.h>
#include <joltc/joltc.h>
#include <joltc/Math/Quat.h>
#include <joltc/Math/Transform.h>
#include <joltc/Math/Vector3.h>
#include <joltc/Physics/Body/BodyCreationSettings.h>
#include <joltc/Physics/Body/BodyID.h>
#include <joltc/Physics/Body/BodyInterface.h>
#include <joltc/Physics/Collision/Shape/Shape.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool LoadMap(Map *map, Asset *mapData)
{
	if (!map || !mapData)
	{
		return false;
	}

	DataReader *reader = CreateDataReaderFromAsset(mapData);

	size_t bytesRemaining = mapData->size;
	size_t strLength = 0;

	JPH_BodyInterface *bodyInterface = JPH_PhysicsSystem_GetBodyInterface(map->physicsSystem);

	EXPECT_BYTES_BOOL(1, bytesRemaining);
	map->renderSky = ReadUint8(reader);
	if (map->renderSky)
	{
		map->skyTexture = ReadStringSafe(reader, &strLength);
		bytesRemaining -= strLength;
		bytesRemaining += sizeof(size_t);
	} else
	{
		map->skyTexture = NULL;
	}
	map->discordRpcIcon = ReadStringSafe(reader, &strLength);
	bytesRemaining -= strLength;
	bytesRemaining += sizeof(size_t);
	map->discordRpcName = ReadStringSafe(reader, &strLength);
	bytesRemaining -= strLength;
	bytesRemaining += sizeof(size_t);

	EXPECT_BYTES_BOOL(sizeof(size_t), bytesRemaining);
	const size_t numActors = ReadSizeT(reader);
	for (size_t i = 0; i < numActors; i++)
	{
		size_t actorClassLength = 0;
		char *actorClass = ReadStringSafe(reader, &actorClassLength);
		bytesRemaining -= actorClassLength;
		bytesRemaining -= sizeof(size_t);

		EXPECT_BYTES_BOOL(sizeof(float) * 6, bytesRemaining);
		Transform xfm;
		xfm.position.x = ReadFloat(reader);
		xfm.position.y = ReadFloat(reader);
		xfm.position.z = ReadFloat(reader);
		const float rotX = ReadFloat(reader);
		const float rotY = ReadFloat(reader);
		const float rotZ = ReadFloat(reader);
		Vector3 eulerAngles = {rotX, rotY, rotZ};
		JPH_Quat_FromEulerAngles(&eulerAngles, &xfm.rotation);

		LockingList ioConnections = {0};
		ListInit(ioConnections, LIST_POINTER);
		EXPECT_BYTES_BOOL(sizeof(size_t), bytesRemaining);
		const size_t numConnections = ReadSizeT(reader);
		for (size_t j = 0; j < numConnections; j++)
		{
			ActorConnection *connection = malloc(sizeof(ActorConnection));
			CheckAlloc(connection);
			connection->sourceActorOutput = ReadStringSafe(reader, &strLength);
			bytesRemaining -= strLength;
			bytesRemaining += sizeof(size_t);
			connection->targetActorName = ReadStringSafe(reader, &strLength);
			bytesRemaining -= strLength;
			bytesRemaining += sizeof(size_t);
			connection->targetActorInput = ReadStringSafe(reader, &strLength);
			bytesRemaining -= strLength;
			bytesRemaining += sizeof(size_t);
			uint8_t hasOverride = ReadUint8(reader);
			// TODO data size validation for params
			if (hasOverride)
			{
				size_t paramSize = ReadParam(reader, &connection->outParamOverride);
				bytesRemaining -= paramSize;
			} else
			{
				connection->outParamOverride.type = PARAM_TYPE_NONE;
			}
			connection->numRefires = ReadSizeT(reader);
			ListAdd(ioConnections, connection);
		}
		KvList params;
		// TODO: Add EXPECT_BYTES for this
		bytesRemaining -= ReadKvList(reader, params);

		if (strcmp(actorClass, "player") == 0)
		{
			SetPlayerTransform(&map->player, &xfm);
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

	EXPECT_BYTES_BOOL(sizeof(size_t), bytesRemaining);
	map->modelCount = ReadSizeT(reader);
	map->models = malloc(sizeof(MapModel) * map->modelCount);
	CheckAlloc(map->models);
	for (size_t i = 0; i < map->modelCount; i++)
	{
		MapModel *model = &map->models[i];
		char *materialName = ReadStringSafe(reader, &strLength);
		bytesRemaining -= sizeof(size_t);
		bytesRemaining -= strLength;
		model->material = LoadMapMaterial(materialName);
		assert(model->material);
		free(materialName);

		EXPECT_BYTES_BOOL(sizeof(uint32_t), bytesRemaining);
		model->vertexCount = ReadUint32(reader);
		model->vertices = malloc(sizeof(MapVertex) * model->vertexCount);
		CheckAlloc(model->vertices);
		for (uint32_t j = 0; j < model->vertexCount; j++)
		{
			MapVertex *vertex = model->vertices + j;
			EXPECT_BYTES_BOOL(sizeof(float) * 7, bytesRemaining);
			vertex->position.x = ReadFloat(reader);
			vertex->position.y = ReadFloat(reader);
			vertex->position.z = ReadFloat(reader);
			vertex->uv.x = ReadFloat(reader);
			vertex->uv.y = ReadFloat(reader);
			vertex->lightmapUv.x = ReadFloat(reader);
			vertex->lightmapUv.y = ReadFloat(reader);
		}
		EXPECT_BYTES_BOOL(sizeof(uint32_t), bytesRemaining);
		model->indexCount = ReadUint32(reader);
		EXPECT_BYTES_BOOL(sizeof(uint32_t) * model->indexCount, bytesRemaining);
		model->indices = malloc(sizeof(uint32_t) * model->indexCount);
		CheckAlloc(model->indices);
		ReadBuffer(reader, sizeof(uint32_t) * model->indexCount, model->indices);
	}


	Transform collisionXfm = {
		.rotation = JPH_Quat_Identity,
	};

	EXPECT_BYTES_BOOL(sizeof(size_t), bytesRemaining);
	const size_t numCollisionMeshes = ReadSizeT(reader);
	for (size_t i = 0; i < numCollisionMeshes; i++)
	{
		EXPECT_BYTES_BOOL((sizeof(float) * 3) + sizeof(size_t), bytesRemaining);
		collisionXfm.position.x = ReadFloat(reader);
		collisionXfm.position.y = ReadFloat(reader);
		collisionXfm.position.z = ReadFloat(reader);
		const size_t subShapeCount = ReadSizeT(reader);
		if (subShapeCount == 0)
		{
			continue;
		}

		JPH_StaticCompoundShapeSettings *compoundShapeSettings = JPH_StaticCompoundShapeSettings_Create();

		for (size_t j = 0; j < subShapeCount; j++)
		{
			ModelStaticCollider staticCollider;
			EXPECT_BYTES_BOOL(sizeof(size_t), bytesRemaining);
			staticCollider.numTriangles = ReadSizeT(reader);
			staticCollider.tris = malloc(sizeof(JPH_Triangle) * staticCollider.numTriangles);
			CheckAlloc(staticCollider.tris);
			for (size_t k = 0; k < staticCollider.numTriangles; k++)
			{
				JPH_Triangle *triangle = &staticCollider.tris[k];
				triangle->materialIndex = 0;
				EXPECT_BYTES_BOOL(sizeof(float) * 9, bytesRemaining);

				triangle->v1.x = ReadFloat(reader);
				triangle->v1.y = ReadFloat(reader);
				triangle->v1.z = ReadFloat(reader);

				triangle->v2.x = ReadFloat(reader);
				triangle->v2.y = ReadFloat(reader);
				triangle->v2.z = ReadFloat(reader);

				triangle->v3.x = ReadFloat(reader);
				triangle->v3.y = ReadFloat(reader);
				triangle->v3.z = ReadFloat(reader);
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
		const JPH_BodyID body = JPH_BodyInterface_CreateAndAddBody(bodyInterface,
																   bodyCreationSettings,
																   JPH_Activation_Activate);
		ListAdd(map->joltBodies, body);
		JPH_BodyCreationSettings_Destroy(bodyCreationSettings);
		JPH_ShapeSettings_Destroy((JPH_ShapeSettings *)compoundShapeSettings);
		JPH_Shape_Destroy(shape);
	}

	JPH_PhysicsSystem_OptimizeBroadPhase(map->physicsSystem);

	EXPECT_BYTES_BOOL(sizeof(size_t) * 2, bytesRemaining);
	map->lightmapWidth = ReadSizeT(reader);
	map->lightmapHeight = ReadSizeT(reader);
	size_t lightmapDataSize = sizeof(uint16_t) *
							  4 *
							  map->lightmapHeight *
							  map->lightmapWidth; // uint16_t because float16
	EXPECT_BYTES_BOOL(lightmapDataSize, bytesRemaining);
	map->lightmapPixels = malloc(lightmapDataSize);
	CheckAlloc(map->lightmapPixels);
	ReadBuffer(reader, lightmapDataSize, map->lightmapPixels);

	EXPECT_BYTES_BOOL(sizeof(uint16_t), bytesRemaining);
	map->lightCount = ReadUint16(reader);
	map->pointLights = malloc(sizeof(Light) * map->lightCount);
	CheckAlloc(map->pointLights);
	EXPECT_BYTES_BOOL((sizeof(float) * 16 + sizeof(uint32_t)) * map->lightCount, bytesRemaining);
	for (size_t i = 0; i < map->lightCount; i++)
	{
		Light *light = &map->pointLights[i];
		light->type = ReadUint32(reader);
		light->transform.position.x = ReadFloat(reader);
		light->transform.position.y = ReadFloat(reader);
		light->transform.position.z = ReadFloat(reader);
		Vector3 rotation;
		rotation.x = ReadFloat(reader);
		rotation.y = ReadFloat(reader);
		rotation.z = ReadFloat(reader);
		JPH_Quat_FromEulerAngles(&rotation, &light->transform.rotation);

		light->color[0] = ReadFloat(reader);
		light->color[1] = ReadFloat(reader);
		light->color[2] = ReadFloat(reader);

		light->brightness = ReadFloat(reader);
		light->constantAttenuation = ReadFloat(reader);
		light->linearAttenuation = ReadFloat(reader);
		light->quadraticAttenuation = ReadFloat(reader);
		light->attenuationMultiplier = ReadFloat(reader);
		light->brightAngle = ReadFloat(reader);
		light->fadingAngle = ReadFloat(reader);
	}

	DestroyDataReader(reader);
	FreeAsset(mapData);

	LoadMapModels(map);

	return map;
}
