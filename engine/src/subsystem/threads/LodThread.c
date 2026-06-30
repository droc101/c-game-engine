//
// Created by NBT22 on 6/13/25.
//

#include <assert.h>
#include <engine/structs/Actor.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/List.h>
#include <engine/subsystem/Logging.h>
#include <engine/subsystem/threads/LodThread.h>
#include <joltc/Math/Vector3.h>
#include <joltc/Physics/Body/BodyInterface.h>
#include <SDL3/SDL_atomic.h>
#include <SDL3/SDL_mutex.h>
#include <SDL3/SDL_thread.h>
#include <stdbool.h>
#include <stddef.h>

static bool shouldExit;
static SDL_Thread *lodThread;
static SDL_Semaphore *canStartSemaphore;
static SDL_AtomicInt canStart;
static SDL_Mutex *mutex;

// ReSharper disable once CppDFAConstantFunctionResult
static int LodThreadMain(void * /*data*/)
{
	while (!shouldExit)
	{
		bool waitResult = SDL_WaitSemaphoreTimeout(canStartSemaphore, 16);
		while (!waitResult)
		{
			if (shouldExit)
			{
				return 0;
			}
			waitResult = SDL_WaitSemaphoreTimeout(canStartSemaphore, 16);
		}

		SDL_LockMutex(mutex);

		const int oldValue = SDL_SetAtomicInt(&canStart, 0);
		assert(oldValue == 1);

		const GlobalState *state = GetState();
		const LockingList *actors = &state->map->actors;
		ListLock(*actors);
		const size_t actorCount = actors->length;
		const float lodMultiplier = state->options.lodMultiplier;
		bool shouldReloadActors = false; // TODO only written to, never read back?
		Vector3 actorPosition = {};
		Vector3 offsetFromCamera = {};
		for (size_t i = 0; i < actorCount; i++)
		{
			Actor *actor = ListGetPointer(*actors, i);
			if (!actor->hasModel || actor->model->lodCount == 1)
			{
				continue;
			}
			JPH_BodyInterface_GetPosition(actor->bodyInterface, actor->bodyId, &actorPosition);
			Vector3_Subtract(&actorPosition, &state->camera->transform.position, &offsetFromCamera);
			const float distanceSquared = Vector3_LengthSquared(&offsetFromCamera);
			while (actor->currentLod != 0 &&
				   actor->model->lods[actor->currentLod].distanceSquared * lodMultiplier > distanceSquared)
			{
				actor->currentLod--;
				shouldReloadActors = true;
			}
			while (actor->model->lodCount > actor->currentLod + 1 &&
				   actor->model->lods[actor->currentLod + 1].distanceSquared * lodMultiplier <= distanceSquared)
			{
				actor->currentLod++;
				shouldReloadActors = true;
			}
		}
		ListUnlock(*actors);
		// if (currentRenderer == RENDERER_VULKAN && !VK_UpdateActors(actors, shouldReloadActors))
		// {
		// 	Error("Failed to load actors!");
		// }

		SDL_UnlockMutex(mutex);
	}
	return 0;
}

void LodThreadInit()
{
	canStartSemaphore = SDL_CreateSemaphore(0);
	mutex = SDL_CreateMutex();
	lodThread = SDL_CreateThread(LodThreadMain, "GameLODThread", NULL);
}

void LodThreadDestroy()
{
	if (shouldExit)
	{
		return;
	}
	LogDebug("Terminating LOD thread...\n");
	shouldExit = true;
	SDL_WaitThread(lodThread, NULL);
	SDL_DestroySemaphore(canStartSemaphore);
	SDL_DestroyMutex(mutex);
}

void SignalLodThreadCanStart()
{
	if (SDL_CompareAndSwapAtomicInt(&canStart, 0, 1))
	{
		SDL_SignalSemaphore(canStartSemaphore);
	}
}

void LockLodThreadMutex()
{
	return SDL_LockMutex(mutex);
}

void UnlockLodThreadMutex()
{
	return SDL_UnlockMutex(mutex);
}
