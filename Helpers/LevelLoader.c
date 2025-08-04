//
// Created by droc101 on 4/21/2024.
//

#include "LevelLoader.h"
#include <stdio.h>
#include "../Helpers/CommonAssets.h"
#include "../Structs/Actor.h"
#include "../Structs/Level.h"
#include "../Structs/Vector2.h"
#include "../Structs/Wall.h"
#include "Core/DataReader.h"
#include "Core/Error.h"
#include "Core/KVList.h"
#include "Core/Logging.h"

/**
 * Prints an error and returns NULL if there are not enough bytes remaining to read
 * @param expected The number of bytes expected to read
 * @note I apologize for this macro and am open to better ideas.
 * @warning This leaks memory if the level is invalid
 */
#define EXPECT_BYTES(expected) \
	{ \
		if (bytesRemaining < (expected)) \
		{ \
			LogError("Not enough bytes remaining to read %zu bytes\n", (expected)); \
			return NULL; \
		} \
		bytesRemaining -= (expected); \
	}

Level *LoadLevel(const byte *data, const size_t dataSize)
{
	Level *level = CreateLevel();
	size_t offset = 0;
	size_t bytesRemaining = dataSize;
	JPH_BodyInterface *bodyInterface = JPH_PhysicsSystem_GetBodyInterface(level->physicsSystem);

	EXPECT_BYTES(32 + sizeof(short) + 1);
	ReadString(data, &offset, level->name, 32);
	level->courseNum = ReadShort(data, &offset);
	level->hasCeiling = ReadByte(data, &offset);

	char levelDataCeilOrSkyTex[64];
	char levelDataFloorTex[64];

	EXPECT_BYTES(64 * 2);
	ReadString(data, &offset, levelDataCeilOrSkyTex, 64);
	ReadString(data, &offset, levelDataFloorTex, 64);


	snprintf(level->ceilOrSkyTex, 80, "texture/%s.gtex", levelDataCeilOrSkyTex);
	snprintf(level->floorTex, 80, "texture/%s.gtex", levelDataFloorTex);

	EXPECT_BYTES(64);
	ReadString(data, &offset, level->music, 64);

	EXPECT_BYTES(sizeof(uint) + sizeof(float) + sizeof(float));
	level->fogColor = ReadUint(data, &offset);
	level->fogStart = ReadFloat(data, &offset);
	level->fogEnd = ReadFloat(data, &offset);

	EXPECT_BYTES(sizeof(float) * 3);
	level->player.transform.position.x = ReadFloat(data, &offset);
	level->player.transform.position.z = ReadFloat(data, &offset);

	level->player.transform.rotation.y = ReadFloat(data, &offset);

	JPH_CharacterVirtual_SetPosition(level->player.joltCharacter, &level->player.transform.position);

	EXPECT_BYTES(sizeof(uint));
	const uint actorCount = ReadUint(data, &offset);
	for (int i = 0; i < actorCount; i++)
	{
		EXPECT_BYTES(sizeof(float) * 3);
		const float actorX = ReadFloat(data, &offset);
		const float actorZ = ReadFloat(data, &offset);
		const float actorRotation = ReadFloat(data, &offset);
		EXPECT_BYTES(sizeof(int) + sizeof(byte) * 4);
		const uint actorType = ReadUint(data, &offset);
		const char actorName[64];
		EXPECT_BYTES(64);
		ReadString(data, &offset, (char *)&actorName, 64);

		KvList params;
		KvListCreate(&params);
		const uint paramCount = ReadUint(data, &offset);
		for (size_t j = 0; j < paramCount; j++)
		{
			char key[64];
			Param param;
			EXPECT_BYTES(64 + sizeof(Param));
			ReadString(data, &offset, (char *)&key, 64);
			ReadBytes(data, &offset, sizeof(Param), &param);
			KvSetUnsafe(&params, key, param);
		}

		Actor *a = CreateActor((Transform[]){{{actorX, 0.0f, actorZ}, {0.0f, actorRotation, 0.0f}}},
							   actorType,
							   &params,
							   bodyInterface);

		EXPECT_BYTES(sizeof(uint));
		const uint connectionCount = ReadUint(data, &offset);
		for (int j = 0; j < connectionCount; j++)
		{
			ActorConnection *ac = malloc(sizeof(ActorConnection));
			CheckAlloc(ac);
			EXPECT_BYTES(1 + 64 + 1 + sizeof(Param));
			ac->myOutput = ReadByte(data, &offset);
			char outActorName[64];
			ReadString(data, &offset, (char *)&outActorName, 64);
			strcpy(ac->outActorName, outActorName);
			ac->targetInput = ReadByte(data, &offset);
			ReadBytes(data, &offset, sizeof(Param), &ac->outParamOverride);
			ListAdd(a->ioConnections, ac);
		}

		ListAdd(level->actors, a);
		if (actorName[0] != '\0')
		{
			NameActor(a, actorName, level);
		}
	}

	EXPECT_BYTES(sizeof(uint));
	const uint wallCount = ReadUint(data, &offset);
	for (int i = 0; i < wallCount; i++)
	{
		EXPECT_BYTES(sizeof(float) * 4);
		const float wallAX = ReadFloat(data, &offset);
		const float wallAY = ReadFloat(data, &offset);
		const float wallBX = ReadFloat(data, &offset);
		const float wallBY = ReadFloat(data, &offset);
		char lDataWallTex[64];
		EXPECT_BYTES(64);
		ReadString(data, &offset, (char *)&lDataWallTex, 64);
		const char wallTex[80];
		snprintf(wallTex, 80, "texture/%s.gtex", lDataWallTex);
		EXPECT_BYTES(sizeof(float) * 2);
		const float wallUVScale = ReadFloat(data, &offset);
		const float wallUVOffset = ReadFloat(data, &offset);
		Wall *w = CreateWall(v2(wallAX, wallAY), v2(wallBX, wallBY), wallTex, wallUVScale, wallUVOffset);
		WallBake(w);
		CreateWallCollider(w, bodyInterface);
		ListAdd(level->walls, w);
	}

	JPH_PhysicsSystem_OptimizeBroadPhase(level->physicsSystem);

	return level;
}
