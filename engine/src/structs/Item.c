//
// Created by droc101 on 1/21/26.
//

#include <engine/structs/Actor.h>
#include <engine/structs/Color.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/Item.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

void DefaultItemConstruct(Item *this)
{
	this->data = NULL;
}

void DefaultItemDestruct(Item *this)
{
	free(this->data);
}

void DefaultItemFixedUpdateFunction(Item *this, GlobalState *state, double delta)
{
	(void)this;
	(void)state;
	(void)delta;
}

void DefaultItemUpdateFunction(Item *this, GlobalState *state)
{
	(void)this;
	state->map->viewmodel.transform.position.y = state->camera->yOffset * 0.2f - 0.35f;
}

bool DefaultItemCanTargetFunction(Item *this, Actor *targetedActor, Color *crosshairColor)
{
	(void)this;
	(void)targetedActor;
	(void)crosshairColor;
	return false;
}

void DefaultItemHudRenderFunction(Item *this)
{
	(void)this;
}

void DefaultItemSwitchToFunction(Item *this, Viewmodel *viewmodel)
{
	(void)this;
	viewmodel->enabled = false;
}

void DefaultItemSwitchFromFunction(Item *this, Viewmodel *viewmodel)
{
	(void)this;
	(void)viewmodel;
}

bool DefaultItemUseFunction(Item *this)
{
	(void)this;
	return false;
}
