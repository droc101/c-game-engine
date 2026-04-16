//
// Created by droc101 on 12/12/24.
//

#include <engine/debug/FrameGrapher.h>
#include <engine/helpers/MathEx.h>
#include <engine/physics/Physics.h>
#include <engine/structs/GameState.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/List.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Input.h>
#include <engine/subsystem/Logging.h>
#include <engine/subsystem/threads/PhysicsThread.h>
#include <engine/subsystem/Timing.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_mutex.h>
#include <SDL3/SDL_thread.h>
#include <SDL3/SDL_timer.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static SDL_Thread *physicsThread;
static SDL_Mutex *physicsThreadMutex;
static SDL_Mutex *physicsTickMutex;

static List physicsThreadInputEventQueue;

/**
 * The function to run in the physics thread
 * @warning Only touch this when you have a lock on the mutex
 */
GameStateFixedUpdateFunction PhysicsThreadFunction;

/**
 * Whether to quit the physics thread on the next iteration
 * @warning Only touch this when you have a lock on the mutex
 */
bool physicsThreadPostQuit = false;

void PhysicsThreadQueueInputEvent(const SDL_Event *event)
{
	SDL_Event *copiedEvent = malloc(sizeof(SDL_Event));
	CheckAlloc(copiedEvent);
	memcpy(copiedEvent, event, sizeof(SDL_Event));
	SDL_LockMutex(physicsThreadMutex);
	ListAdd(physicsThreadInputEventQueue, copiedEvent);
	SDL_UnlockMutex(physicsThreadMutex);
}

/**
 * The main function for the physics thread
 * @return 0
 */
int PhysicsThreadMain(void * /*data*/)
{
	double lastTickTime = PHYSICS_TARGET_NS_D;
	while (true)
	{
		const uint64_t timeStart = GetTimeNs();
		SDL_LockMutex(physicsThreadMutex);
		SDL_LockMutex(physicsTickMutex);
		if (physicsThreadPostQuit)
		{
			SDL_UnlockMutex(physicsThreadMutex);
			SDL_UnlockMutex(physicsTickMutex);
			return 0;
		}

		for (size_t i = 0; i < physicsThreadInputEventQueue.length; i++)
		{
			SDL_Event *event = ListGetPointer(physicsThreadInputEventQueue, i);
			InputSystemProcessEvent(physicsThreadInput, event);
			free(event);
		}
		ListClear(physicsThreadInputEventQueue);

		if (PhysicsThreadFunction == NULL)
		{
			GetState()->physicsFrame++;
			SDL_UnlockMutex(physicsThreadMutex);
			SDL_UnlockMutex(physicsTickMutex);
			SDL_DelayPrecise(PHYSICS_TARGET_NS); // pls only spin if needed 🥺
			UpdateInputStates(physicsThreadInput);
			continue;
		}
		// The function is copied to a local variable so we can unlock the mutex during its runtime
		const GameStateFixedUpdateFunction UpdateFunction = PhysicsThreadFunction;
		SDL_UnlockMutex(physicsThreadMutex);

		// delta is the portion of one "tick" that the last tick took (including idle time)
		// ticks should be around 1/60th of a second
		const double delta = lastTickTime / PHYSICS_TARGET_NS_D;
		UpdateFunction(GetState(), delta);
		UpdateInputStates(physicsThreadInput);
		GetState()->physicsFrame++;
		SDL_UnlockMutex(physicsTickMutex);

		uint64_t timeEnd = GetTimeNs();
		uint64_t timeElapsed = timeEnd - timeStart;
		SDL_DelayPrecise(PHYSICS_TARGET_NS - timeElapsed);
		timeEnd = GetTimeNs();
		timeElapsed = timeEnd - timeStart;
		TickGraphUpdate(timeElapsed);
		lastTickTime = min(PHYSICS_MIN_NS_D, (double)timeElapsed);
	}
}

void PhysicsThreadInit()
{
	LogDebug("Initializing physics thread...\n");
	ListInit(physicsThreadInputEventQueue, LIST_POINTER);
	PhysicsThreadFunction = NULL;
	physicsThreadPostQuit = false;
	physicsThreadMutex = SDL_CreateMutex();
	physicsTickMutex = SDL_CreateMutex();
	physicsThread = SDL_CreateThread(PhysicsThreadMain, "GamePhysics", NULL);
	if (physicsThread == NULL)
	{
		const char *error = SDL_GetError();
		LogError("Failed to create physics thread: %s\n", error);
		Error("Failed to create physics thread");
	}
}

void PhysicsThreadSetFunction(const GameStateFixedUpdateFunction function)
{
	SDL_LockMutex(physicsThreadMutex);
	GetState()->physicsFrame = 0;
	PhysicsThreadFunction = function;
	SDL_UnlockMutex(physicsThreadMutex);
}

void PhysicsThreadTerminate()
{
	LogDebug("Terminating physics thread...\n");
	SDL_LockMutex(physicsThreadMutex);
	physicsThreadPostQuit = true;
	SDL_UnlockMutex(physicsThreadMutex);
	SDL_WaitThread(physicsThread, NULL);
	ListAndContentsFree(physicsThreadInputEventQueue);
	SDL_DestroyMutex(physicsThreadMutex);
	SDL_DestroyMutex(physicsTickMutex);
}

void PhysicsThreadLockTickMutex()
{
	SDL_LockMutex(physicsTickMutex);
}

void PhysicsThreadUnlockTickMutex()
{
	SDL_UnlockMutex(physicsTickMutex);
}
