//
// Created by droc101 on 12/12/24.
//

#include <engine/debug/FrameGrapher.h>
#include <engine/helpers/MathEx.h>
#include <engine/physics/Physics.h>
#include <engine/structs/GlobalState.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Input.h>
#include <engine/subsystem/Logging.h>
#include <engine/subsystem/threads/PhysicsThread.h>
#include <engine/subsystem/Timing.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_mutex.h>
#include <SDL3/SDL_thread.h>
#include <SDL3/SDL_timer.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static SDL_Thread *physicsThread;
static SDL_Mutex *physicsThreadMutex;
static SDL_Mutex *physicsTickMutex;
static SDL_Semaphore *physicsTickHasEnded;

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
		SDL_TryWaitSemaphore(physicsTickHasEnded); // TODO this returns something
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
			GetState()->physicsFrame++;
			SDL_UnlockMutex(physicsThreadMutex);
			SDL_UnlockMutex(physicsTickMutex);
			SDL_DelayPrecise(PHYSICS_TARGET_NS); // pls no spin 🥺
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
		SDL_SignalSemaphore(physicsTickHasEnded);

		uint64_t timeEnd = GetTimeNs();
		uint64_t timeElapsed = timeEnd - timeStart;
		if (timeElapsed < PHYSICS_TARGET_NS)
		{
			const uint64_t delayNs = (PHYSICS_TARGET_NS - timeElapsed);
			SDL_DelayPrecise(delayNs);
		}
		timeEnd = GetTimeNs();
		timeElapsed = timeEnd - timeStart;
		TickGraphUpdate(timeElapsed);
		lastTickTime = min(PHYSICS_MIN_NS_D, (double)timeElapsed);
	}
}

void PhysicsThreadInit()
{
	LogDebug("Initializing physics thread...\n");
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
	GetState()->physicsFrame = 0;
	PhysicsThreadFunction = function;
	SDL_UnlockMutex(physicsThreadMutex);
	if (function)
	{
		if (SDL_TryWaitSemaphore(physicsTickHasEnded) &&
			SDL_WaitSemaphoreTimeout(physicsTickHasEnded, 1000)) // TODO are these checks correct
		{
			LogError("Failed to wait for physics tick semaphore with error %s", SDL_GetError());
			Error("Failed to wait for physics tick semaphore!");
		}
	}
}

void PhysicsThreadTerminate()
{
	LogDebug("Terminating physics thread...\n");
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
