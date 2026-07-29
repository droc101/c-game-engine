//
// Created by droc101 on 7/27/26.
//

#include <engine/actor/prop/WorldText.h>
#include <engine/graphics/Drawing.h>
#include <engine/graphics/Font.h>
#include <engine/graphics/RenderingHelpers.h>
#include <engine/structs/Actor.h>
#include <engine/structs/ActorDefinition.h>
#include <engine/structs/Camera.h>
#include <engine/structs/Color.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/KVList.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Error.h>
#include <joltc/Math/RVec3.h>
#include <joltc/Math/Transform.h>
#include <joltc/Physics/Body/BodyInterface.h>
#include <SDL3/SDL_stdinc.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct WorldTextData
{
	Color backgroundColor;
	Color textColor;
	uint32_t size;
	char *text;
	float visibleDistance;
} WorldTextData;

static void WorldTextInit(Actor *this, const KvList params, Transform *transform)
{
	this->hasModel = false;
	this->wall = NULL;
	ActorCreateEmptyBody(this, transform);

	WorldTextData *data = malloc(sizeof(WorldTextData));
	CheckAlloc(data);
	data->backgroundColor = KvGetColor(params, "background_color", COLOR(0x80000000));
	data->textColor = KvGetColor(params, "text_color", COLOR_WHITE);
	data->size = KvGetInt(params, "font_size", 16);
	data->text = strdup(KvGetString(params, "text", "Hello, World!"));
	data->visibleDistance = KvGetFloat(params, "visible_distance", 80);

	this->extraData = data;
}

static void WorldTextRenderUi(Actor *this)
{
	Camera *cam = GetState()->camera;
	if (cam)
	{
		const WorldTextData *data = this->extraData;
		JPH_RVec3 position;
		JPH_BodyInterface_GetPosition(this->bodyInterface, this->bodyId, &position);

		const float dst = glm_vec3_distance((float *)&position, (float *)&cam->transform.position);
		if (dst > data->visibleDistance)
		{
			return;
		}

		const Vector2 screenPosition = ProjectPosition((float *)&position, cam);
		const Vector2 textSize = MeasureText(data->text, data->size, smallFont);
		const Vector2 textPosition = v2(screenPosition.x - textSize.x / 2, screenPosition.y - textSize.y / 2);
		DrawRect(textPosition.x - 4, textPosition.y - 4, textSize.x + 8, textSize.y + 8, data->backgroundColor);
		FontDrawString(textPosition, data->text, data->size, data->textColor, smallFont);
	}
}

static void WorldTextDestroy(Actor *this)
{
	const WorldTextData *data = this->extraData;
	free(data->text);
}

ActorDefinition worldTextActorDefinition = {
	.Update = DefaultActorUpdate,
	.OnPlayerContactAdded = DefaultActorOnPlayerContactAdded,
	.OnPlayerContactPersisted = DefaultActorOnPlayerContactPersisted,
	.OnPlayerContactRemoved = DefaultActorOnPlayerContactRemoved,
	.RenderUi = WorldTextRenderUi,
	.Interact = DefaultActorInteract,
	.Destroy = WorldTextDestroy,
	.Init = WorldTextInit,
};

void RegisterWorldText()
{
	RegisterDefaultActorInputs(&worldTextActorDefinition);
	RegisterActor(WORLD_TEXT_ACTOR_NAME, &worldTextActorDefinition);
}
