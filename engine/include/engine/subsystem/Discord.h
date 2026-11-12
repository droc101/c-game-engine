//
// Created by droc101 on 10/4/25.
//

/**
 * @file Discord.h
 * Discord Rich Presence subsystem
 */

#ifndef GAME_DISCORD_H
#define GAME_DISCORD_H

/// Discord RPC state
typedef enum RPCState
{
	/// Unknown state
	UNKNOWN,
	/// In main menu or subscreen
	IN_MENUS,
	/// In map, unpaused
	IN_GAME,
	/// Paused in map
	PAUSED,
} RPCState;

/**
 * Initialize the Discord Game SDK
 * @private
 */
void DiscordInit();

/**
 * Run Discord Game SDK update functions
 * @private
 */
void DiscordUpdate();

/**
 * Update the Rich Presence status
 * @private
 */
void DiscordUpdateRPC();

/**
 * Cleanup & destroy the Discord Game SDK instance
 * @private
 */
void DiscordDestroy();

#endif //GAME_DISCORD_H
