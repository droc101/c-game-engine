//
// Created by NBT22 on 6/13/25.
//

#include "LodThread.h"
#include "../../defines.h"
#include "../../Structs/GlobalState.h"
#include "../Core/Error.h"
#include "Vulkan/Vulkan.h"

static bool shouldExit;
static SDL_Thread *lodThread;
static SDL_sem *canStart;
static SDL_sem *hasEnded;
static SDL_mutex *mutex;

// ReSharper disable once CppDFAConstantFunctionResult
int LodThreadMain(void * /*data*/)
{
	while (!shouldExit)
	{
		int waitResult = SDL_SemWaitTimeout(canStart, 16);
		while (waitResult != 0)
		{
			if (shouldExit)
			{
				return 0;
			}
			if (waitResult == SDL_MUTEX_TIMEDOUT)
			{
				waitResult = SDL_SemWaitTimeout(canStart, 16);
			} else
			{
				Error("Failed to wait for LOD thread start semaphore!");
			}
		}

		if (SDL_LockMutex(mutex) != 0)
		{
			Error("Failed to lock LOD thread mutex!");
		}

		const GlobalState *state = GetState();
		const LockingList *actors = &state->level->actors;
		const size_t actorCount = actors->length;
		const float lodMultiplier = state->options.lodMultiplier;
		bool shouldReloadActors = false;
		Vector3 actorPosition = {};
		Vector3 offsetFromPlayer = {};
		for (size_t i = 0; i < actorCount; i++)
		{
			Actor *actor = ListGetPointer(*actors, i);
			if (!actor->actorModel || actor->actorModel->lodCount == 1)
			{
				continue;
			}
			JPH_BodyInterface_GetPosition(actor->bodyInterface, actor->bodyId, &actorPosition);
			Vector3_Subtract(&actorPosition, &state->level->player.transform.position, &offsetFromPlayer);
			const float distanceSquared = Vector3_LengthSquared(&offsetFromPlayer);
			while (actor->currentLod != 0 &&
				   actor->actorModel->lods[actor->currentLod]->distanceSquared * lodMultiplier > distanceSquared)
			{
				actor->currentLod--;
				shouldReloadActors = true;
			}
			while (actor->actorModel->lodCount > actor->currentLod + 1 &&
				   actor->actorModel->lods[actor->currentLod + 1]->distanceSquared * lodMultiplier <= distanceSquared)
			{
				actor->currentLod++;
				shouldReloadActors = true;
			}
		}
		if (currentRenderer == RENDERER_VULKAN && !VK_UpdateActors(actors, shouldReloadActors))
		{
			Error("Failed to load actors!");
		}

		if (SDL_UnlockMutex(mutex) != 0)
		{
			Error("Failed to unlock LOD thread mutex!");
		}

		if (SDL_SemPost(hasEnded) != 0)
		{
			Error("Failed to signal LOD thread end semaphore!");
		}
	}
	return 0;
}

void LodThreadInit()
{
	canStart = SDL_CreateSemaphore(0);
	hasEnded = SDL_CreateSemaphore(1);
	mutex = SDL_CreateMutex();
	lodThread = SDL_CreateThread(LodThreadMain, "GameLODThread", NULL);
}

void LodThreadDestroy()
{
	if (shouldExit)
	{
		return;
	}
	shouldExit = true;
	SDL_WaitThread(lodThread, NULL);
	SDL_DestroySemaphore(canStart);
	SDL_DestroySemaphore(hasEnded);
	SDL_DestroyMutex(mutex);
}

int SignalLodThreadCanStart()
{
	return SDL_SemPost(canStart);
}

int WaitForLodThreadToEnd()
{
	return SDL_SemWait(hasEnded);
}

int LockLodThreadMutex()
{
	return SDL_LockMutex(mutex);
}

int UnlockLodThreadMutex()
{
	return SDL_UnlockMutex(mutex);
}
