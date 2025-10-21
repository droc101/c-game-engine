//
// Created by droc101 on 10/4/25.
//

#ifndef GAME_DISCORD_H
#define GAME_DISCORD_H

typedef enum RPCState
{
	UNKNOWN,
	IN_MENUS,
	IN_GAME,
	PAUSED
} RPCState;

/**
 * Initialize the Discord Game SDK
 */
void DiscordInit();

/**
 * Run Discord Game SDK update functions
 */
void DiscordUpdate();

/**
 * Update the Rich Presence status
 */
void DiscordUpdateRPC();

/**
 * Cleanup & destroy the Discord Game SDK instance
 */
void DiscordDestroy();

#endif //GAME_DISCORD_H
