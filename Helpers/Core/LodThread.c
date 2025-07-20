//
// Created by NBT22 on 6/13/25.
//

#include "LodThread.h"
#include "../../defines.h"
#include "../../Structs/GlobalState.h"
#include "../../Structs/Vector2.h"
#include "../Graphics/Vulkan/Vulkan.h"
#include "Error.h"

static bool shouldExit;
static SDL_Thread *lodThread;
static SDL_sem *canStart;
static SDL_sem *hasEnded;
static SDL_mutex *mutex;

// ReSharper disable once CppDFAConstantFunctionResult
int LodThreadMain(void *)
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

		const Vector2 playerPosition = GetState()->level->player.pos;
		const List actors = GetState()->level->actors;
		const size_t actorCount = actors.length;
		const float lodMultiplier = GetState()->options.lodMultiplier;
		switch (currentRenderer)
		{
			case RENDERER_VULKAN:
				bool shouldReloadActors = false;
				for (size_t i = 0; i < actorCount; i++)
				{
					Actor *actor = ListGet(actors, i);
					if (!actor->actorModel)
					{
						continue;
					}
					const float distance = Vector2Distance(actor->position, playerPosition);
					while (actor->currentLod != 0 &&
						   actor->actorModel->lods[actor->currentLod]->distance * lodMultiplier > distance)
					{
						actor->currentLod--;
						shouldReloadActors = true;
					}
					while (actor->actorModel->lodCount > actor->currentLod + 1 &&
						   actor->actorModel->lods[actor->currentLod + 1]->distance * lodMultiplier <= distance)
					{
						actor->currentLod++;
						shouldReloadActors = true;
					}
				}
				if (!VK_UpdateActors(&actors, shouldReloadActors))
				{
					Error("Failed to load actors!");
				}
				break;
			case RENDERER_OPENGL:
			default:
				for (size_t i = 0; i < actorCount; i++)
				{
					Actor *actor = ListGet(actors, i);
					if (!actor->actorModel)
					{
						continue;
					}
					const float distance = Vector2Distance(actor->position, playerPosition);
					while (actor->currentLod != 0 &&
						   actor->actorModel->lods[actor->currentLod]->distance * lodMultiplier > distance)
					{
						actor->currentLod--;
					}
					while (actor->actorModel->lodCount > actor->currentLod + 1 &&
						   actor->actorModel->lods[actor->currentLod + 1]->distance * lodMultiplier <= distance)
					{
						actor->currentLod++;
					}
				}
				break;
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
	hasEnded = SDL_CreateSemaphore(0);
	mutex = SDL_CreateMutex();
	lodThread = SDL_CreateThread(LodThreadMain, "LOD Thread", NULL);
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
