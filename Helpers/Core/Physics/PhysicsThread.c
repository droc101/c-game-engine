//
// Created by droc101 on 12/12/24.
//

#include "PhysicsThread.h"
#include <SDL_error.h>
#include <SDL_mutex.h>
#include <SDL_thread.h>
#include <SDL_timer.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "../../../Debug/FrameGrapher.h"
#include "../../../Structs/GlobalState.h"
#include "../Error.h"
#include "../Input.h"
#include "../Logging.h"
#include "../Timing.h"
#include "Physics.h"

static SDL_Thread *physicsThread;
static SDL_mutex *physicsThreadMutex;
static SDL_mutex *physicsTickMutex;
static SDL_sem *physicsTickHasEnded;

/**
 * The function to run in the physics thread
 * @warning Only touch this when you have a lock on the mutex
 */
FixedUpdateFunction PhysicsThreadFunction;

/**
 * Whether to quit the physics thread on the next iteration
 * @warning Only touch this when you have a lock on the mutex
 */
bool physicsThreadPostQuit = false;

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
		SDL_SemTryWait(physicsTickHasEnded);
		SDL_LockMutex(physicsThreadMutex);
		SDL_LockMutex(physicsTickMutex);
		if (physicsThreadPostQuit)
		{
			SDL_UnlockMutex(physicsThreadMutex);
			SDL_UnlockMutex(physicsTickMutex);
			return 0;
		}
		InputPhysicsTickBegin();
		if (PhysicsThreadFunction == NULL)
		{
			SDL_UnlockMutex(physicsThreadMutex);
			SDL_UnlockMutex(physicsTickMutex);
			SDL_Delay(PHYSICS_TARGET_MS); // pls no spin 🥺
			GetState()->physicsFrame++;
			continue;
		}
		// The function is copied to a local variable so we can unlock the mutex during its runtime
		const FixedUpdateFunction UpdateFunction = PhysicsThreadFunction;
		SDL_UnlockMutex(physicsThreadMutex);

		// delta is the portion of one "tick" that the last tick took (including idle time)
		// ticks should be around 1/60th of a second
		const double delta = lastTickTime / PHYSICS_TARGET_NS_D;
		UpdateFunction(GetState(), delta);
		GetState()->physicsFrame++;
		SDL_UnlockMutex(physicsTickMutex);
		SDL_SemPost(physicsTickHasEnded);

		uint64_t timeEnd = GetTimeNs();
		uint64_t timeElapsed = timeEnd - timeStart;
		if (timeElapsed < PHYSICS_TARGET_NS)
		{
			const uint64_t delayMs = (PHYSICS_TARGET_NS - timeElapsed) / 1000000;
			SDL_Delay(delayMs);
		}
		timeEnd = GetTimeNs();
		timeElapsed = timeEnd - timeStart;
		TickGraphUpdate(timeElapsed);
		lastTickTime = (double)timeElapsed;
	}
}

void PhysicsThreadInit()
{
	PhysicsThreadFunction = NULL;
	physicsThreadPostQuit = false;
	physicsThreadMutex = SDL_CreateMutex();
	physicsTickMutex = SDL_CreateMutex();
	physicsTickHasEnded = SDL_CreateSemaphore(0);
	physicsThread = SDL_CreateThread(PhysicsThreadMain, "GamePhysics", NULL);
	if (physicsThread == NULL)
	{
		const char *error = SDL_GetError();
		LogError("Failed to create physics thread: %s\n", error);
		Error("Failed to create physics thread");
	}
}

void PhysicsThreadSetFunction(const FixedUpdateFunction function)
{
	SDL_LockMutex(physicsThreadMutex);
	PhysicsThreadFunction = function;
	SDL_UnlockMutex(physicsThreadMutex);
	if (function)
	{
		if (SDL_SemTryWait(physicsTickHasEnded) == SDL_MUTEX_TIMEDOUT &&
			SDL_SemWaitTimeout(physicsTickHasEnded, 1000) < 0)
		{
			LogError("Failed to wait for physics tick semaphore with error %s", SDL_GetError());
			Error("Failed to wait for physics tick semaphore!");
		}
	}
}

void PhysicsThreadTerminate()
{
	SDL_LockMutex(physicsThreadMutex);
	physicsThreadPostQuit = true;
	SDL_UnlockMutex(physicsThreadMutex);
	SDL_WaitThread(physicsThread, NULL);
	SDL_DestroyMutex(physicsThreadMutex);
	SDL_DestroyMutex(physicsTickMutex);
	SDL_DestroySemaphore(physicsTickHasEnded);
}

void PhysicsThreadLockTickMutex()
{
	SDL_LockMutex(physicsTickMutex);
}

void PhysicsThreadUnlockTickMutex()
{
	SDL_UnlockMutex(physicsTickMutex);
}
