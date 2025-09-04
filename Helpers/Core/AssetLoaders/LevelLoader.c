//
// Created by droc101 on 4/21/2024.
//

#include "LevelLoader.h"
#include <joltc/joltc.h>
#include <joltc/Physics/Body/BodyInterface.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../../Structs/Actor.h"
#include "../../../Structs/ActorDefinition.h"
#include "../../../Structs/Level.h"
#include "../../../Structs/Param.h"
#include "../../../Structs/Vector2.h"
#include "../../../Structs/Wall.h"
#include "../AssetReader.h"
#include "../DataReader.h"
#include "../Error.h"
#include "../KVList.h"
#include "../List.h"
#include "../Logging.h"

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

Level *LoadLevel(const uint8_t *data, const size_t dataSize)
{
	Level *level = CreateLevel();
	size_t offset = 0;
	size_t bytesRemaining = dataSize;
	JPH_BodyInterface *bodyInterface = JPH_PhysicsSystem_GetBodyInterface(level->physicsSystem);

	EXPECT_BYTES(32 + sizeof(short) + 1);
	offset += sizeof(char) * 32; // discard level name
	offset += sizeof(short); // discard course number
	level->hasCeiling = ReadByte(data, &offset);

	EXPECT_BYTES(64 * 2);
	// char levelDataCeilOrSkyTex[64];
	// char levelDataFloorTex[64];
	// ReadString(data, &offset, levelDataCeilOrSkyTex, 64);
	// ReadString(data, &offset, levelDataFloorTex, 64);
	// snprintf(level->ceilOrSkyTex, 80, TEXTURE("%s"), levelDataCeilOrSkyTex);
	// snprintf(level->floorTex, 80, TEXTURE("%s"), levelDataFloorTex);

	offset += sizeof(char) * 64 * 2;
	snprintf(level->ceilOrSkyTex, 80, level->hasCeiling ? TEXTURE("level/ceiling_test") : TEXTURE("level/sky_test"));
	snprintf(level->floorTex, 80, TEXTURE("level/floor_test"));

	EXPECT_BYTES(64);
	ReadString(data, &offset, level->music, 64);

	EXPECT_BYTES(sizeof(uint32_t) + sizeof(float) + sizeof(float));
	level->fogColor = ReadUint(data, &offset);
	level->fogStart = ReadFloat(data, &offset);
	level->fogEnd = ReadFloat(data, &offset);

	EXPECT_BYTES(sizeof(float) * 3);
	level->player.transform.position.x = ReadFloat(data, &offset);
	level->player.transform.position.z = ReadFloat(data, &offset);

	level->player.transform.rotation.y = ReadFloat(data, &offset);

	JPH_CharacterVirtual_SetPosition(level->player.joltCharacter, &level->player.transform.position);

	EXPECT_BYTES(sizeof(uint32_t));
	const uint32_t actorCount = ReadUint(data, &offset);
	for (uint32_t i = 0; i < actorCount; i++)
	{
		EXPECT_BYTES(sizeof(float) * 3);
		const float actorX = ReadFloat(data, &offset);
		const float actorZ = ReadFloat(data, &offset);
		const float actorRotation = ReadFloat(data, &offset);
		EXPECT_BYTES(sizeof(int) + sizeof(uint8_t) * 4);
		const uint32_t actorType = ReadUint(data, &offset);
		char actorName[64] = {0};
		EXPECT_BYTES(64);
		ReadString(data, &offset, actorName, 64);

		KvList params;
		KvListCreate(params);
		const uint32_t paramCount = ReadUint(data, &offset);
		for (size_t j = 0; j < paramCount; j++)
		{
			char key[64];
			Param param;
			EXPECT_BYTES(64 + sizeof(Param));
			ReadString(data, &offset, key, 64);
			ReadBytes(data, &offset, sizeof(Param), &param);
			KvSetUnsafe(params, key, param);
		}

		const char *actorTypeString = "";
		switch (actorType - 1)
		{
			case ACTOR_TYPE_TEST:
				actorTypeString = TEST_ACTOR_NAME;
				break;
			case ACTOR_TYPE_COIN:
				actorTypeString = COIN_ACTOR_NAME;
				break;
			case ACTOR_TYPE_GOAL:
				actorTypeString = GOAL_ACTOR_NAME;
				break;
			case ACTOR_TYPE_DOOR:
				actorTypeString = DOOR_ACTOR_NAME;
				break;
			case ACTOR_TYPE_TRIGGER:
				actorTypeString = TRIGGER_ACTOR_NAME;
				break;
			case ACTOR_TYPE_IO_PROXY:
				actorTypeString = IO_PROXY_ACTOR_NAME;
				break;
			case ACTOR_TYPE_PHYSBOX:
				actorTypeString = PHYSBOX_ACTOR_NAME;
				break;
			case ACTOR_TYPE_LASER:
				actorTypeString = LASER_ACTOR_NAME;
				break;
			case ACTOR_TYPE_STATIC_MODEL:
				actorTypeString = STATIC_MODEL_ACTOR_NAME;
				break;
			case ACTOR_TYPE_SOUND_PLAYER:
				actorTypeString = SOUND_PLAYER_ACTOR_NAME;
				break;
			case ACTOR_TYPE_SPRITE:
				actorTypeString = SPRITE_ACTOR_NAME;
				break;
			case ACTOR_TYPE_LASER_EMITTER:
				actorTypeString = LASER_EMITTER_ACTOR_NAME;
				break;
			case ACTOR_TYPE_LOGIC_BINARY:
			case UINT32_MAX:
				actorTypeString = LOGIC_BINARY_ACTOR_NAME;
				break;
			case ACTOR_TYPE_LOGIC_DECIMAL:
				actorTypeString = LOGIC_DECIMAL_ACTOR_NAME;
				break;
			case ACTOR_TYPE_LOGIC_COUNTER:
				actorTypeString = LOGIC_COUNTER_ACTOR_NAME;
				break;
			default:
				break;
		}
		Actor *a = CreateActor((Transform[]){{{actorX, 0.0f, actorZ}, {0.0f, actorRotation, 0.0f}}},
							   actorTypeString,
							   params,
							   bodyInterface);

		EXPECT_BYTES(sizeof(uint32_t));
		const uint32_t connectionCount = ReadUint(data, &offset);
		for (uint32_t j = 0; j < connectionCount; j++)
		{
			ActorConnection *ac = malloc(sizeof(ActorConnection));
			CheckAlloc(ac);
			EXPECT_BYTES(1 + 64 + 1 + sizeof(Param));
			ac->myOutput = ReadByte(data, &offset);
			char outActorName[64];
			ReadString(data, &offset, outActorName, 64);
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

	EXPECT_BYTES(sizeof(uint32_t));
	const uint32_t wallCount = ReadUint(data, &offset);
	for (uint32_t i = 0; i < wallCount; i++)
	{
		EXPECT_BYTES(sizeof(float) * 4);
		const float wallAX = ReadFloat(data, &offset);
		const float wallAY = ReadFloat(data, &offset);
		const float wallBX = ReadFloat(data, &offset);
		const float wallBY = ReadFloat(data, &offset);

		EXPECT_BYTES(64);
		offset += sizeof(char) * 64;
		// char lDataWallTex[64];
		// ReadString(data, &offset, lDataWallTex, 64);
		char wallTex[80];
		// snprintf(wallTex, 80, TEXTURE("%s"), lDataWallTex);
		snprintf(wallTex, 80, TEXTURE("level/wall_test"));
		EXPECT_BYTES(sizeof(float) * 2);
		const float wallUVScale = ReadFloat(data, &offset);
		const float wallUVOffset = ReadFloat(data, &offset);
		Wall *wall = CreateWall(v2(wallAX, wallAY), v2(wallBX, wallBY), wallTex, wallUVScale, wallUVOffset);
		WallBake(wall);
		if (wall->dx == 0 && wall->dy == 0)
		{
			continue;
		}
		CreateWallCollider(wall, bodyInterface);
		ListAdd(level->walls, wall);
	}

	JPH_PhysicsSystem_OptimizeBroadPhase(level->physicsSystem);

	return level;
}
