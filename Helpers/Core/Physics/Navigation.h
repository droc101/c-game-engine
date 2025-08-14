//
// Created by noah on 2/8/25.
//

#ifndef NAVIGATION_H
#define NAVIGATION_H

#include <stdbool.h>
#include <stdint.h>
#include "../../../Structs/Actor.h"
#include "../../../Structs/Vector2.h"

typedef struct NavigationConfig NavigationConfig;

typedef void (*ActorIdleFunction)(Actor *this, double delta);

typedef void (*ActorTargetReachedFunction)(Actor *this, double delta);

/**
 * A struct that holds configuration and state values for the navigation system
 */
struct NavigationConfig
{
	/// The field of view of the actor; used for determining if the target is visible
	float fov;
	/// The speed at which the actor will move along the path
	/// @note This value is used to scale a unit vector that points in the direction the actor needs to move
	float speed;
	/// A value from zero to one for the speed at which the actor will rotate to look in the direction of the target
	/// @note This value is used as the linear interpolation factor for rotation
	float rotationSpeed;
	/// A value from one to zero that indicates how directly the actor will move towards the target
	/// One is move directly to the target, and zero is the most random movement while still going towards the target
	float directness;
	/// The distance from the target that the actor is allowed to reach
	/// @warning Due to momentum the actor will get closer to the target than this value,
	///  potentially by a significant amount
	float minDistance;
	/// The distance that the actor can see the target from when idle
	float agroDistance;
	/// The distance that the actor can see the target from once already agro locked
	float deAgroDistance;
	/// The number of physics ticks that it will take for the actor to forget about the target
	/// once the line of sight is broken
	uint32_t agroTicks;
	/// The function to call when the actor does not see the target and has no current agro
	ActorIdleFunction IdleFunction;
	/// The function to call when the actor has reached @c minDistance from the target
	ActorTargetReachedFunction TargetReachedFunction;

	/// The number of ticks remaining until the actor forgets about the target
	/// @note This is reset to @c agroTicks every time the actor sees the target
	double agroTicksRemaining;
	/// When line of sight is broken, this value is used to navigate to the last place the target was known to be
	/// @note This is reset to the target's position any time the actor sees the target
	Vector2 lastKnownTarget;
	/// A relative angle (in radians) to add to the angle from the actor to the target,
	/// which is used to make the pathing less direct
	float directionModifier;
	/// The number of ticks until the value in @c directionModifier will be randomized again
	/// @note This is a random value from 10 to 60
	double ticksUntilDirectionChange;
};

/**
 * Gets the relative angle between the actor's rotation and the player position
 * @param actor The actor to get the angle relative to
 * @return The relative angle between the actor's rotation and the player position
 */
float PlayerRelativeAngle(const Actor *actor);

/**
 * Check an actor's vision (defined by @c navigationConfig) for the player
 * @param actor The actor to check player visibility for
 * @param navigationConfig The navigation configuration
 * @return If the player is visible
 */
bool IsPlayerVisible(const Actor *actor, NavigationConfig navigationConfig);

/**
 * Move an actor either towards a target or run an idle or target reached function
 * @param actor The actor to move
 * @param navigationConfig The navigation configuration
 * @param delta A value from zero to one that is a fraction of how long the physics tick took compared to the target
 */
void NavigationStep(Actor *actor, NavigationConfig *navigationConfig, double delta);

#endif //NAVIGATION_H
