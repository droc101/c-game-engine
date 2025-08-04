//
// Created by droc101 on 4/22/2024.
//

#ifndef GAME_ACTOR_H
#define GAME_ACTOR_H

#include "../defines.h"

#define ACTOR_KILL_INPUT 0

#define ACTOR_SPAWN_OUTPUT 1
#define ACTOR_KILLED_OUTPUT 0

/**
 * Create an Actor
 * @param transform Actor position
 * @param actorType Actor type
 * @param params Parameters for the actor, can be NULL
 * @param bodyInterface The Joly body interface within which to create the actor's rigid body
 * @return Initialized Actor struct
 */
Actor *CreateActor(const Transform *transform, ActorType actorType, KvList *params, JPH_BodyInterface *bodyInterface);

/**
 * Destroy an Actor
 * @param actor actor to destroy
 */
void FreeActor(Actor *actor);

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
