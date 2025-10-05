//
// Created by droc101 on 10/4/25.
//

#ifndef GAME_DISCORD_H
#define GAME_DISCORD_H

typedef enum RPCState
{
	IN_MENUS,
	IN_GAME,
	PAUSED
} RPCState;

void DiscordInit();

void DiscordUpdate();

void DiscordUpdateRPC();

void DiscordDestroy();

#endif //GAME_DISCORD_H
