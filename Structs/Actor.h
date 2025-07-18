//
// Created by droc101 on 4/22/2024.
//

#ifndef GAME_ACTOR_H
#define GAME_ACTOR_H

#include "../defines.h"

#define ACTOR_KILL_INPUT 0

#define ACTOR_SPAWN_OUTPUT 1
#define ACTOR_KILLED_OUTPUT 0

typedef enum ActorType ActorType;

enum ActorType
{
	EMPTY_ACTOR,
	TEST_ACTOR,
	COIN_ACTOR,
	GOAL_ACTOR,
	DOOR_ACTOR,
	TRIGGER_ACTOR,
	IO_PROXY_ACTOR,
	PHYSBOX_ACTOR,
	LASER_ACTOR,
	STATIC_MODEL_ACTOR,
	SOUND_PLAYER_ACTOR,
	SPRITE_ACTOR,
	LASER_EMITTER_ACTOR,
};

/**
 * Create an Actor
 * @param position Actor position
 * @param rotation Actor rotation
 * @param actorType Actor type
 * @param params Parameters for the actor, can be NULL
 * @param worldId The Box2D world within which to create the actor
 * @return Initialized Actor struct
 */
Actor *CreateActor(Vector2 position, float rotation, uint actorType, KvList *params, b2WorldId worldId);

/**
 * Destroy an Actor
 * @param actor actor to destroy
 */
void FreeActor(Actor *actor);

/**
 * 
 * @param this The actor to create the collider for
 * @param worldId The world within which to create the collider
 */
void CreateActorWallCollider(Actor *this, b2WorldId worldId);

/**
 * Directly trigger an input on an actor
 * @param sender The actor sending the signal
 * @param receiver The actor receiving the signal
 * @param signal The signal to send
 * @param param The parameter to send with the signal
 */
void ActorTriggerInput(const Actor *sender, const Actor *receiver, const byte signal, const Param *param);

/**
 * Fire signal from an actor
 * @param sender The actor sending the signal
 * @param signal The signal to send
 * @param defaultParam The default parameter to send with the signal
 */
void ActorFireOutput(const Actor *sender, byte signal, Param defaultParam);

/**
 * Destroy an actor connection
 * @param connection The connection to destroy
 */
void DestroyActorConnection(ActorConnection *connection);

/**
 * Default signal handler for actors, handling global signals such as kill
 * @return Whether the signal was handled
 */
bool DefaultSignalHandler(Actor *this, const Actor *sender, byte signal, const Param *param);

#endif //GAME_ACTOR_H
