//
// Created by droc101 on 4/21/2024.
//

#include "LevelLoader.h"
#include <box2d/box2d.h>
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
	Level *l = CreateLevel();
	size_t offset = 0;
	size_t bytesRemaining = dataSize;

	EXPECT_BYTES(32 + sizeof(short) + 1);
	ReadString(data, &offset, l->name, 32);
	l->courseNum = ReadShort(data, &offset);
	l->hasCeiling = ReadByte(data, &offset);

	char lDataCeilOrSkyTex[64];
	char lDataFloorTex[64];

	EXPECT_BYTES(128);
	ReadString(data, &offset, lDataCeilOrSkyTex, 64);
	ReadString(data, &offset, lDataFloorTex, 64);


	snprintf(l->ceilOrSkyTex, 80, "texture/%s.gtex", lDataCeilOrSkyTex);
	snprintf(l->floorTex, 80, "texture/%s.gtex", lDataFloorTex);

	EXPECT_BYTES(64);
	ReadString(data, &offset, l->music, 64);

	EXPECT_BYTES(sizeof(uint) + sizeof(float) + sizeof(float));
	l->fogColor = ReadUint(data, &offset);
	l->fogStart = ReadFloat(data, &offset);
	l->fogEnd = ReadFloat(data, &offset);

	EXPECT_BYTES(sizeof(float) * 3);
	l->player.pos.x = ReadFloat(data, &offset);
	l->player.pos.y = ReadFloat(data, &offset);
	l->player.angle = ReadFloat(data, &offset);

	b2Body_SetTransform(l->player.bodyId, l->player.pos, b2MakeRot(l->player.angle));

	EXPECT_BYTES(sizeof(uint));
	const uint actorCount = ReadUint(data, &offset);
	for (int i = 0; i < actorCount; i++)
	{
		EXPECT_BYTES(sizeof(float) * 3);
		const float actorX = ReadFloat(data, &offset);
		const float actorY = ReadFloat(data, &offset);
		const float actorRotation = ReadFloat(data, &offset);
		EXPECT_BYTES(sizeof(int) + sizeof(byte) * 4);
		const int actorType = ReadInt(data, &offset);
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

		Actor *a = CreateActor(v2(actorX, actorY), actorRotation, actorType, &params, l->worldId);

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
			ListAdd(&a->ioConnections, ac);
		}

		ListAdd(&l->actors, a);
		if (actorName[0] != '\0')
		{
			NameActor(a, actorName, l);
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
		CreateWallCollider(w, l->worldId);
		ListAdd(&l->walls, w);
	}

	return l;
}
