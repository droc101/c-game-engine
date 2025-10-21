//
// Created by droc101 on 10/4/25.
//

#ifdef ENABLE_DISCORD_SDK

#include <discord_game_sdk.h>
#include <engine/assets/GameConfigLoader.h>
#include <engine/structs/GlobalState.h>
#include <engine/subsystem/Discord.h>
#include <engine/subsystem/Logging.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

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
	params.client_id = (DiscordClientId)config.discordAppId;
	params.flags = DiscordCreateFlags_NoRequireDiscord;
	params.event_data = &app;

	const enum EDiscordResult res = DiscordCreate(DISCORD_VERSION, &params, &app.core);
	if (res != DiscordResult_Ok)
	{
		DiscordDestroy();
		LogError("Failed to start Discord Game SDK: Errno %d\n", res);
		return;
	}

	app.activityManager = app.core->get_activity_manager(app.core);

	GetState()->rpcState = UNKNOWN;

	DiscordUpdateRPC();

	LogInfo("Discord Game SDK started with App ID %zu\n", config.discordAppId);
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
	activity.application_id = (DiscordClientId)config.discordAppId;
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

void DiscordDestroy()
{
	if (app.core)
	{
		LogDebug("Cleaning up Discord Game SDK...\n");
		app.activityManager->clear_activity(app.activityManager, NULL, NULL);
		app.core->destroy(app.core);
		app.core = NULL;
		app.activityManager = NULL;
	}
}

#else
void DiscordInit() {}
void DiscordUpdate() {}
void DiscordUpdateRPC() {}
void DiscordDestroy() {}
#endif
