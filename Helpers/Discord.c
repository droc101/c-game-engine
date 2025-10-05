//
// Created by droc101 on 10/4/25.
//

#ifdef ENABLE_DISCORD_SDK

#include "Discord.h"
#include <discord_game_sdk.h>
#include <stdio.h>
#include "../Structs/GlobalState.h"
#include "Core/AssetLoaders/GameConfigLoader.h"
#include "Core/Logging.h"

struct DiscordApplication
{
	struct IDiscordCore *core;
	struct IDiscordActivityManager *activityManager;
	DiscordUserId user_id;
};

struct DiscordApplication app;

void DiscordInit()
{
	memset(&app, 0, sizeof(app));

	if (config.discordAppId == 0)
	{
		return;
	}

	struct DiscordCreateParams params;
	params.client_id = config.discordAppId;
	params.flags = DiscordCreateFlags_Default;
	params.event_data = &app;

	enum EDiscordResult res = DiscordCreate(DISCORD_VERSION, &params, &app.core);
	if (res != DiscordResult_Ok)
	{
		app.core = NULL;
		LogError("Failed to start Discord Game SDK: Errno %d\n", res);
		return;
	}

	app.activityManager = app.core->get_activity_manager(app.core);

	DiscordUpdateRPC();

	LogInfo("Discord Game SDK started\n");
}

void DiscordUpdate()
{
	if (!app.core)
	{
		return;
	}
	app.core->run_callbacks(app.core);
}

static void ActivityCallback(void * /*data*/, const enum EDiscordResult result)
{
	if (result != DiscordResult_Ok)
	{
		LogError("Failed to set Discord RPC: Errno %d\n", result);
	}
}

void DiscordUpdateRPC()
{
	if (!app.core || !app.activityManager)
	{
		return;
	}
	struct DiscordActivity activity = {0};
	activity.application_id = config.discordAppId;
	activity.type = DiscordActivityType_Playing;
	switch (GetState()->rpcState)
	{
		case IN_GAME:
			snprintf(activity.assets.large_text, 128, "Playing %s", GetState()->levelName);
			snprintf(activity.details, 128, "Playing %s", GetState()->levelName);
			snprintf(activity.assets.large_image, 128, "level_%s", GetState()->levelName);
			break;
		case PAUSED:
			snprintf(activity.assets.large_text, 128, "Playing %s", GetState()->levelName);
			snprintf(activity.details, 128, "Game Paused");
			snprintf(activity.assets.large_image, 128, "level_%s", GetState()->levelName);
			break;
		case IN_MENUS:
		default:
			strcpy(activity.assets.large_image, "logo");
			snprintf(activity.details, 128, "In the menus");
			strcpy(activity.assets.large_text, "GAME");
			break;
	}
	app.activityManager->update_activity(app.activityManager, &activity, NULL, ActivityCallback);
}

#else
void DiscordInit() {}
void DiscordUpdate() {}
void DiscordUpdateRPC() {}
#endif
