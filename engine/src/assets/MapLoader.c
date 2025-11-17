//
// Created by droc101 on 4/21/2024.
//

#include <engine/assets/AssetReader.h>
#include <engine/assets/DataReader.h>
#include <engine/assets/MapLoader.h>
#include <engine/structs/Actor.h>
#include <engine/structs/ActorDefinition.h>
#include <engine/structs/KVList.h>
#include <engine/structs/List.h>
#include <engine/structs/Map.h>
#include <engine/structs/Param.h>
#include <engine/structs/Vector2.h>
#include <engine/structs/Wall.h>
#include <engine/subsystem/Logging.h>
#include <joltc/joltc.h>
#include <joltc/Math/Quat.h>
#include <joltc/Math/Vector3.h>
#include <joltc/Physics/Body/BodyInterface.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <engine/actor/IoProxy.h>
#include <engine/actor/LogicBinary.h>
#include <engine/actor/LogicCounter.h>
#include <engine/actor/LogicDecimal.h>
#include <engine/actor/SoundPlayer.h>
#include <engine/actor/Sprite.h>
#include <engine/actor/StaticModel.h>
#include <engine/actor/Trigger.h>

Map *LoadMap(const char *path)
{
	Map *level = CreateLevel();
	Asset *levelData = DecompressAsset(path, false);
	size_t offset = 0;
	JPH_BodyInterface *bodyInterface = JPH_PhysicsSystem_GetBodyInterface(level->physicsSystem);

	snprintf(level->skyTexture, 80, TEXTURE("level/sky_test"));

	const size_t numActors = ReadSizeT(levelData->data, &offset);
	for (size_t i = 0; i < numActors; i++)
	{
		size_t actorClassLength = 0;
		char *actorClass = ReadStringSafe(levelData->data, &offset, levelData->size, &actorClassLength);
		Transform xfm;
		xfm.position.x = ReadFloat(levelData->data, &offset);
		xfm.position.y = ReadFloat(levelData->data, &offset);
		xfm.position.z = ReadFloat(levelData->data, &offset);
		const float rotX = ReadFloat(levelData->data, &offset);
		const float rotY = ReadFloat(levelData->data, &offset);
		const float rotZ = ReadFloat(levelData->data, &offset);
		Vector3 eulerAngles = {rotX, rotY, rotZ};
		JPH_Quat_FromEulerAngles(&eulerAngles, &xfm.rotation);
		LockingList ioConnections = {0};
		ListInit(ioConnections, LIST_POINTER);
		const size_t numConnections = ReadSizeT(levelData->data, &offset);
		for (size_t j = 0; j < numConnections; j++)
		{
			ActorConnection *conn = malloc(sizeof(ActorConnection));
			conn->sourceActorOutput = ReadStringSafe(levelData->data, &offset, levelData->size, NULL);
			conn->targetActorName = ReadStringSafe(levelData->data, &offset, levelData->size, NULL);
			conn->targetActorInput = ReadStringSafe(levelData->data, &offset, levelData->size, NULL);
			uint8_t hasOverride = ReadByte(levelData->data, &offset);
			ReadParam(levelData->data, levelData->size, &offset, &conn->outParamOverride);
			ListAdd(ioConnections, conn);
		}
		KvList params;
		KvListCreate(params);
		size_t numParams = ReadSizeT(levelData->data, &offset);
		for (size_t j = 0; j < numParams; j++)
		{
			char *key = ReadStringSafe(levelData->data, &offset, levelData->size, NULL);
			Param p;
			ReadParam(levelData->data, levelData->size, &offset, &p);
			KvSetUnsafe(params, key, p);
		}

		if (strcmp(actorClass, "player") == 0)
		{
			level->player.transform = xfm;
			// TODO free stuff
			continue;
		}

		Actor *a = CreateActor(&xfm, actorClass, params, bodyInterface);
		a->ioConnections = ioConnections;
		ListAdd(level->actors, a);
	}

	JPH_PhysicsSystem_OptimizeBroadPhase(level->physicsSystem);

	return level;
}
