//
// Created by droc101 on 4/21/2024.
//

#include <assert.h>
#include <engine/assets/AssetReader.h>
#include <engine/assets/DataReader.h>
#include <engine/assets/MapLoader.h>
#include <engine/assets/MapMaterialLoader.h>
#include <engine/graphics/RenderingHelpers.h>
#include <engine/helpers/MathEx.h>
#include <engine/physics/Physics.h>
#include <engine/physics/PlayerPhysics.h>
#include <engine/structs/Actor.h>
#include <engine/structs/ActorDefinition.h>
#include <engine/structs/ActorWall.h>
#include <engine/structs/Asset.h>
#include <engine/structs/Color.h>
#include <engine/structs/KVList.h>
#include <engine/structs/Light.h>
#include <engine/structs/List.h>
#include <engine/structs/Map.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Logging.h>
#include <joltc/constants.h>
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

	if (mapData->typeVersion != MAP_ASSET_VERSION)
	{
		LogError("Failed to load map from asset due to version mismatch (got %d, expected %d)\n",
				 mapData->typeVersion,
				 MAP_ASSET_VERSION);
		return false;
	}

	DataReader *reader = CreateDataReaderFromAsset(mapData);

	size_t bytesRemaining = mapData->size;

	JPH_BodyInterface *bodyInterface = JPH_PhysicsSystem_GetBodyInterface(map->physicsSystem);

	EXPECT_BYTES_BOOL(1, bytesRemaining);
	map->renderSky = ReadUint8(reader);
	if (map->renderSky)
	{
		map->skyTexture = ReadStringSafe(reader, &bytesRemaining);
		if (!map->skyTexture)
		{
			LogError("Failed to read map sky texture!\n");
			return false;
		}
	} else
	{
		map->skyTexture = NULL;
	}
	map->discordRpcIcon = ReadStringSafe(reader, &bytesRemaining);
	if (!map->discordRpcIcon)
	{
		LogError("Failed to read Discord RPC icon\n");
		return false;
	}
	map->discordRpcName = ReadStringSafe(reader, &bytesRemaining);
	if (!map->discordRpcName)
	{
		LogError("Failed to read Discord RPC name\n");
		return false;
	}

	EXPECT_BYTES_BOOL(sizeof(size_t), bytesRemaining);
	const size_t numActors = ReadSizeT(reader);
	for (size_t i = 0; i < numActors; i++)
	{
		char *actorClass = ReadStringSafe(reader, &bytesRemaining);
		if (!actorClass)
		{
			LogError("Failed to read actor classname\n");
			return false;
		}

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
			connection->sourceActorOutput = ReadStringSafe(reader, &bytesRemaining);
			if (!connection->sourceActorOutput)
			{
				LogError("Failed to read I/O connection output name\n");
				return false;
			}
			connection->targetActorName = ReadStringSafe(reader, &bytesRemaining);
			if (!connection->targetActorName)
			{
				LogError("Failed to read I/O connection target name\n");
				return false;
			}
			connection->targetActorInput = ReadStringSafe(reader, &bytesRemaining);
			if (!connection->targetActorInput)
			{
				LogError("Failed to read I/O connection input name\n");
				return false;
			}
			EXPECT_BYTES_BOOL(1, bytesRemaining);
			uint8_t hasOverride = ReadUint8(reader);
			if (hasOverride)
			{
				if (!ReadParam(reader, &connection->outParamOverride, &bytesRemaining))
				{
					return false;
				}
			} else
			{
				connection->outParamOverride.type = PARAM_TYPE_NONE;
			}
			EXPECT_BYTES_BOOL(sizeof(size_t), bytesRemaining);
			connection->numRefires = ReadSizeT(reader);
			EXPECT_BYTES_BOOL(sizeof(uint16_t), bytesRemaining);
			connection->delayMs = ReadUint16(reader);
			ListAdd(ioConnections, connection);
		}
		KvList params;
		if (!ReadKvList(reader, params, &bytesRemaining))
		{
			return false;
		}

		if (strcmp(actorClass, "player") == 0)
		{
			map->player.playerCamera.nearPlane = KvGetFloat(params, "near_plane", DEFAULT_NEAR_PLANE);
			map->player.playerCamera.farPlane = KvGetFloat(params, "far_plane", DEFAULT_FAR_PLANE);
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
	map->models = calloc(map->modelCount, sizeof(MapModel));
	CheckAlloc(map->models);
	for (size_t i = 0; i < map->modelCount; i++)
	{
		MapModel *model = &map->models[i];
		char *materialName = ReadStringSafe(reader, &bytesRemaining);
		if (!materialName)
		{
			LogError("Failed to read map material name\n");
			return false;
		}
		model->material = LoadMapMaterial(materialName);
		assert(model->material); // TODO: This should probably be handled better
		free(materialName);

		EXPECT_BYTES_BOOL(sizeof(float) * 6, bytesRemaining);
		model->center.x = ReadFloat(reader);
		model->center.y = ReadFloat(reader);
		model->center.z = ReadFloat(reader);
		model->halfExtent.x = ReadFloat(reader);
		model->halfExtent.y = ReadFloat(reader);
		model->halfExtent.z = ReadFloat(reader);

		EXPECT_BYTES_BOOL(sizeof(uint32_t), bytesRemaining);
		model->vertexCount = ReadUint32(reader);
		model->vertices = malloc(sizeof(MapVertex) * model->vertexCount);
		CheckAlloc(model->vertices);
		for (uint32_t j = 0; j < model->vertexCount; j++)
		{
			MapVertex *vertex = model->vertices + j;
			EXPECT_BYTES_BOOL(sizeof(float) * 10, bytesRemaining);
			vertex->position.x = ReadFloat(reader);
			vertex->position.y = ReadFloat(reader);
			vertex->position.z = ReadFloat(reader);
			vertex->uv.x = ReadFloat(reader);
			vertex->uv.y = ReadFloat(reader);
			vertex->lightmapUv.x = ReadFloat(reader);
			vertex->lightmapUv.y = ReadFloat(reader);
			vertex->normal.x = ReadFloat(reader);
			vertex->normal.y = ReadFloat(reader);
			vertex->normal.z = ReadFloat(reader);
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
		EXPECT_BYTES_BOOL((sizeof(float) * 6), bytesRemaining);
		collisionXfm.position.x = ReadFloat(reader);
		collisionXfm.position.y = ReadFloat(reader);
		collisionXfm.position.z = ReadFloat(reader);
		Vector3 rotationEuler;
		rotationEuler.x = ReadFloat(reader);
		rotationEuler.y = ReadFloat(reader);
		rotationEuler.z = ReadFloat(reader);
		JPH_Quat_FromEulerAngles(&rotationEuler, &collisionXfm.rotation);

		EXPECT_BYTES_BOOL(sizeof(size_t), bytesRemaining);
		const size_t vertexCount = ReadSizeT(reader);
		EXPECT_BYTES_BOOL(sizeof(Vector3) * vertexCount, bytesRemaining);
		Vector3 *vertices = malloc(sizeof(Vector3) * vertexCount);
		CheckAlloc(vertices);
		for (size_t k = 0; k < vertexCount; k++)
		{
			Vector3 *vertex = &vertices[k];
			vertex->x = ReadFloat(reader);
			vertex->y = ReadFloat(reader);
			vertex->z = ReadFloat(reader);
		}
		JPH_Shape *hullShape = (JPH_Shape *)JPH_ConvexHullShape_Create(vertices, vertexCount, JPH_DefaultConvexRadius);
		free(vertices);

		JPH_BodyCreationSettings *bodyCreationSettings = JPH_BodyCreationSettings_Create2_GAME(hullShape,
																							   &collisionXfm,
																							   JPH_MotionType_Static,
																							   OBJECT_LAYER_STATIC,
																							   0);
		JPH_BodyCreationSettings_SetFriction(bodyCreationSettings, 68.0f);
		const JPH_BodyID body = JPH_BodyInterface_CreateAndAddBody(bodyInterface,
																   bodyCreationSettings,
																   JPH_Activation_Activate);
		ListAdd(map->joltBodies, body);
		JPH_BodyCreationSettings_Destroy(bodyCreationSettings);
		JPH_Shape_Destroy(hullShape);
	}

	JPH_PhysicsSystem_OptimizeBroadPhase(map->physicsSystem);

	EXPECT_BYTES_BOOL(sizeof(float), bytesRemaining);
	map->maxInboundsDistance = ReadFloat(reader);
	map->player.playerCamera.farPlane = min(map->player.playerCamera.farPlane, map->maxInboundsDistance);

	EXPECT_BYTES_BOOL(sizeof(size_t) * 2, bytesRemaining);
	map->lightmapWidth = ReadSizeT(reader);
	map->lightmapHeight = ReadSizeT(reader);
	size_t lightmapDataSize = sizeof(_Float16) * 4 * map->lightmapHeight * map->lightmapWidth;
	EXPECT_BYTES_BOOL(lightmapDataSize, bytesRemaining);
	map->lightmapPixels = malloc(lightmapDataSize);
	CheckAlloc(map->lightmapPixels);
	ReadBuffer(reader, lightmapDataSize, map->lightmapPixels);
	EXPECT_BYTES_BOOL(lightmapDataSize, bytesRemaining);
	map->indirectLightmapPixels = malloc(lightmapDataSize);
	CheckAlloc(map->indirectLightmapPixels);
	ReadBuffer(reader, lightmapDataSize, map->indirectLightmapPixels);

	EXPECT_BYTES_BOOL(sizeof(uint32_t), bytesRemaining);
	map->lightCount = ReadUint32(reader);
	map->lights = calloc(map->lightCount, sizeof(Light));
	CheckAlloc(map->lights);
	for (size_t i = 0; i < map->lightCount; i++)
	{
		KvList lightParams;
		if (!ReadKvList(reader, lightParams, &bytesRemaining))
		{
			return false;
		}
		Light *light = &map->lights[i];

		light->type = KvGetByte(lightParams, "type", LIGHT_TYPE_POINT);
		light->position = KvGetVec3(lightParams, "position", (Vector3){0, 0, 0});
		light->rotation = KvGetVec3(lightParams, "rotation", (Vector3){0, 0, 0});
		light->color = KvGetColor(lightParams, "color", COLOR_WHITE);
		light->brightness = KvGetFloat(lightParams, "brightness", 1.0f);

		if (light->type != LIGHT_TYPE_DIRECTIONAL)
		{
			light->constantAttenuation = KvGetFloat(lightParams, "constant_attenuation", 0.0f);
			light->linearAttenuation = KvGetFloat(lightParams, "linear_attenuation", 0.0f);
			light->quadraticAttenuation = KvGetFloat(lightParams, "quadratic_attenuation", 1.0f);
			light->attenuationMultiplier = KvGetFloat(lightParams, "attenuation_multiplier", 2.0f);
			if (light->type == LIGHT_TYPE_SPOT)
			{
				light->brightAngle = KvGetFloat(lightParams, "bright_angle", 30.0f);
				light->fadingAngle = KvGetFloat(lightParams, "fading_angle", 60.0f);
				light->cookie = strdup(KvGetString(lightParams, "cookie", ""));
			}
		} else
		{
			map->directionalLight = light;
		}

		KvListDestroy(lightParams);
	}

	EXPECT_EOF_BYTES(bytesRemaining);

	DestroyDataReader(reader);
	FreeAsset(mapData);

	LoadMapModels(map);

	return true;
}
