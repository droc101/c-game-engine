//
// Created by droc101 on 10/4/26.
//

#include <engine/actor/logic/LogicAchievement.h>
#include <engine/structs/Achievements.h>
#include <engine/structs/Actor.h>
#include <engine/structs/ActorDefinition.h>
#include <engine/structs/KVList.h>
#include <engine/subsystem/Error.h>
#include <joltc/Math/Transform.h>
#include <stdlib.h>
#include <string.h>

typedef struct LogicAchievementData
{
	char *achName;
} LogicAchievementData;

static void LogicAchievementGrantHandler(Actor *this, const Actor * /*sender*/, const Param * /*param*/)
{
	const LogicAchievementData *data = (LogicAchievementData *)this->extraData;
	UnlockAchievement(data->achName);
}

static void LogicAchievementInit(Actor *this, const KvList params, const Transform * /*transform*/)
{
	this->extraData = malloc(sizeof(LogicAchievementData));
	CheckAlloc(this->extraData);
	LogicAchievementData *data = this->extraData;
	data->achName = strdup(KvGetString(params, "achievement", ""));
}

static void LogicAchievementDestroy(Actor *this)
{
	const LogicAchievementData *data = this->extraData;
	free(data->achName);
}

ActorDefinition logicAchievementActorDefinition = {
	.Init = LogicAchievementInit,
	.Destroy = LogicAchievementDestroy,
};

void RegisterLogicAchievement()
{
	RegisterDefaultActorInputs(&logicAchievementActorDefinition);
	RegisterActorInput(&logicAchievementActorDefinition, LOGIC_ACHIEVEMENT_INPUT_GRANT, LogicAchievementGrantHandler);
	RegisterActor(LOGIC_ACHIEVEMENT_ACTOR_NAME, &logicAchievementActorDefinition);
}
