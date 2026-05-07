//
// Created by NBT22 on 10/20/25.
//

#include "helpers/GameActorRegistration.h"
#include "actor/item/ItemEraser.h"
#include "actor/npc/NpcJohn.h"
#include "actor/npc/TestActor.h"
#include "actor/prop/Coin.h"
#include "actor/prop/Door.h"
#include "actor/prop/Goal.h"
#include "actor/prop/Laser.h"
#include "actor/prop/LaserEmitter.h"
#include "actor/prop/Physbox.h"

void RegisterGameActors()
{
	RegisterCoin();
	RegisterDoor();
	RegisterGoal();
	RegisterLaser();
	RegisterLaserEmitter();
	RegisterPhysbox();
	RegisterTestActor();
	RegisterNpcJohn();
	RegisterItemEraser();
}
