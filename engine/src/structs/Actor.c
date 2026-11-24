//
// Created by droc101 on 4/22/2024.
//

#include <assert.h>
#include <engine/physics/Physics.h>
#include <engine/structs/Actor.h>
#include <engine/structs/ActorDefinition.h>
#include <engine/structs/Color.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/KVList.h>
#include <engine/structs/List.h>
#include <engine/structs/Map.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Logging.h>
#include <joltc/enums.h>
#include <joltc/joltc.h>
#include <joltc/Math/Quat.h>
#include <joltc/Math/Transform.h>
#include <joltc/Math/Vector3.h>
#include <joltc/Physics/Body/BodyCreationSettings.h>
#include <joltc/Physics/Body/BodyID.h>
#include <joltc/Physics/Body/BodyInterface.h>
#include <joltc/Physics/Collision/Shape/Shape.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

Actor *CreateActor(Transform *transform, const char *actorType, KvList params, JPH_BodyInterface *bodyInterface)
{
	Actor *actor = calloc(1, sizeof(Actor));
	CheckAlloc(actor);
	actor->definition = GetActorDefinition(actorType);
	actor->visible = true;
	actor->modColor = COLOR_WHITE;
	actor->bodyInterface = bodyInterface;
	JPH_MutableCompoundShapeSettings *shapeSettings = JPH_MutableCompoundShapeSettings_Create();
	actor->shape = (JPH_Shape *)JPH_MutableCompoundShape_Create(shapeSettings);
	JPH_ShapeSettings_Destroy((JPH_ShapeSettings *)shapeSettings);
	actor->bodyId = JPH_BodyId_InvalidBodyID;
	actor->previousTickStartTransform = *transform;
	actor->previousTickEndTransform = *transform;
	ListInit(actor->ioConnections, LIST_POINTER);

	actor->definition->Init(actor, params, transform); // kindly allow the Actor to initialize itself
	ActorFireOutput(actor, ACTOR_OUTPUT_SPAWNED, PARAM_NONE);

	if (params)
	{
		KvListDestroy(params);
	}
	return actor;
}

void FreeActor(Actor *actor)
{
	if (actor->definition->Destroy)
	{
		actor->definition->Destroy(actor);
	}
	if (!actor->hasModel && actor->wall != NULL)
	{
		free(actor->wall->texture);
		free(actor->wall);
		actor->wall = NULL;
	}
	free(actor->extraData);
	actor->extraData = NULL;
	if (actor->bodyId != JPH_BodyId_InvalidBodyID && actor->bodyInterface != NULL)
	{
		JPH_BodyInterface_RemoveAndDestroyBody(actor->bodyInterface, actor->bodyId);
	}
	for (size_t i = 0; i < actor->ioConnections.length; i++)
	{
		ActorConnection *connection = ListGetPointer(actor->ioConnections, i);
		DestroyActorConnection(connection);
	}
	ListFree(actor->ioConnections);
	free(actor);
	actor = NULL;
}

void TeleportActor(Actor *actor, const Vector3 *position, const JPH_Activation activation)
{
	JPH_BodyInterface_SetPosition(actor->bodyInterface, actor->bodyId, position, activation);
	actor->previousTickStartTransform.position = *position;
	actor->previousTickEndTransform.position = *position;
	actor->deltaPosition = Vector3_Zero;
}

void ActorTriggerInput(const Actor *sender, Actor *receiver, const char *input, const Param *param)
{
	if (sender)
	{
		LogDebug("Triggering input \"%s\" on actor %p (%s) from actor %p (%s)\n",
				 input,
				 receiver,
				 receiver->definition->className,
				 sender,
				 sender->definition->className);
	} else
	{
		LogDebug("Triggering input \"%s\" on actor %p (%s)\n", input, receiver, receiver->definition->className);
	}
	const ActorInputHandlerFunction handler = GetActorInputHandler(receiver->definition, input);
	if (handler)
	{
		handler(receiver, sender, param);
	} else
	{
		LogWarning("Could not send signal %s to actor %p because it has no handler!\n", input, receiver);
	}
}

void ActorFireOutput(Actor *sender, const char *output, const Param defaultParam)
{
	ListLock(sender->ioConnections);
	for (size_t i = 0; i < sender->ioConnections.length; i++)
	{
		ActorConnection *connection = ListGetPointer(sender->ioConnections, i);
		if (strcmp(connection->sourceActorOutput, output) == 0)
		{
			const Param *param = &defaultParam;
			if (connection->outParamOverride.type != PARAM_TYPE_NONE)
			{
				param = &connection->outParamOverride;
			}
			QueueIOConnection(GetState()->map, sender, connection, param);

			// connections that have 0 refires at this point have infinite refires
			if (connection->numRefires > 0)
			{
				connection->numRefires--;
				if (connection->numRefires == 0)
				{
					// get vaporized idiot
					DestroyActorConnection(connection);
					ListRemoveAt(sender->ioConnections, i);
					i--; // Make sure not to skip the next connection
				}
			}
		}
	}
	ListUnlock(sender->ioConnections);
}

void DestroyActorConnection(ActorConnection *connection)
{
	free(connection->targetActorName);
	free(connection->sourceActorOutput);
	free(connection->targetActorInput);
	FreeParam(&connection->outParamOverride);
	free(connection);
}

void ActorSignalKill(Actor *this, const Actor * /*sender*/, const Param * /*param*/)
{
	RemoveActor(this);
}

void ActorSignalCancelQueuedIo(Actor *this, const Actor * /*sender*/, const Param * /*param*/)
{
	Map *map = GetState()->map;
	ListLock(map->ioQueue);
	for (int i = (int)map->ioQueue.length - 1; i >= 0; i--)
	{
		QueuedIoConnection *connection = ListGetPointer(map->ioQueue, i);
		if (connection->source == this)
		{
			connection->processed = true; // this causes it to be skipped and freed
		}
	}
	ListUnlock(map->ioQueue);
}

void ActorCreateEmptyBody(Actor *this, const Transform *transform)
{
	JPH_BodyCreationSettings *bodyCreationSettings = JPH_BodyCreationSettings_Create2_GAME(this->shape,
																						   transform,
																						   JPH_MotionType_Static,
																						   OBJECT_LAYER_STATIC,
																						   this);
	this->bodyId = JPH_BodyInterface_CreateAndAddBody(this->bodyInterface,
													  bodyCreationSettings,
													  JPH_Activation_DontActivate);
	JPH_BodyCreationSettings_Destroy(bodyCreationSettings);
}

uint32_t ActorAddShape(Actor *this, const JPH_Shape *shape)
{
	const Transform transform = {
		.position = Vector3_Zero,
		.rotation = JPH_Quat_Identity,
	};
	return ActorAddShapeWithTransform(this, shape, &transform);
}

uint32_t ActorAddShapeWithTransform(Actor *this, const JPH_Shape *shape, const Transform *transform)
{
	const JPH_BodyLockInterface *lockInterface = JPH_PhysicsSystem_GetBodyLockInterface(GetState()->map->physicsSystem);

	JPH_BodyLockWrite writeLock;
	JPH_BodyLockInterface_LockWrite(lockInterface, this->bodyId, &writeLock);
	const uint32_t index = JPH_MutableCompoundShape_AddShape((JPH_MutableCompoundShape *)this->shape,
															 &transform->position,
															 &transform->rotation,
															 shape,
															 0,
															 UINT32_MAX);
	JPH_BodyLockInterface_UnlockWrite(lockInterface, &writeLock);

	JPH_BodyInterface_NotifyShapeChanged(this->bodyInterface,
										 this->bodyId,
										 &Vector3_Zero,
										 false,
										 JPH_Activation_DontActivate);

	return index;
}

void ActorSetParent(Actor *child, Actor *parent)
{
	Transform parentTransform;
	JPH_BodyInterface_GetPositionAndRotation(parent->bodyInterface,
											 parent->bodyId,
											 &parentTransform.position,
											 &parentTransform.rotation);

	JPH_BodyInterface_GetPositionAndRotation(child->bodyInterface,
											 child->bodyId,
											 &child->offsetFromParent.position,
											 &child->offsetFromParent.rotation);
	Vector3_Subtract(&child->offsetFromParent.position, &parentTransform.position, &child->offsetFromParent.position);
	JPH_Quat_Inversed(&parentTransform.rotation, &parentTransform.rotation);
	JPH_Quat_Rotate(&parentTransform.rotation, &child->offsetFromParent.position, &child->offsetFromParent.position);
	JPH_Quat_Multiply(&parentTransform.rotation, &child->offsetFromParent.rotation, &child->offsetFromParent.rotation);

	child->subShapeIndex = ActorAddShapeWithTransform(parent, (JPH_Shape *)child->shape, &child->offsetFromParent);

	JPH_BodyInterface_SetObjectLayer(child->bodyInterface, child->bodyId, OBJECT_LAYER_NO_COLLISION);
	JPH_BodyInterface_SetMotionType(child->bodyInterface,
									child->bodyId,
									JPH_MotionType_Static,
									JPH_Activation_DontActivate);

	child->parent = parent;
	child->previousTickStartTransform = child->previousTickEndTransform;
}

void ActorRemoveParent(Actor *child)
{
	assert(child->parent);

	JPH_MutableCompoundShape_RemoveShape((JPH_MutableCompoundShape *)child->parent->shape, child->subShapeIndex);

	Vector3 parentPosition;
	JPH_BodyInterface_GetPosition(child->parent->bodyInterface, child->parent->bodyId, &parentPosition);
	Vector3 childPosition;
	JPH_BodyInterface_GetPosition(child->bodyInterface, child->bodyId, &childPosition);

	Vector3_Add(&childPosition, &parentPosition, &childPosition);
	TeleportActor(child, &childPosition, JPH_Activation_Activate);
	// TODO: This could technically turn a static object into a dynamic one
	JPH_BodyInterface_SetObjectLayer(child->bodyInterface, child->bodyId, OBJECT_LAYER_DYNAMIC);
	JPH_BodyInterface_SetMotionType(child->bodyInterface,
									child->bodyId,
									JPH_MotionType_Dynamic,
									JPH_Activation_Activate);

	child->parent = NULL;
}
