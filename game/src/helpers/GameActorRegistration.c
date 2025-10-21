//
// Created by NBT22 on 10/20/25.
//

#include "helpers/GameActorRegistration.h"
#include "actor/Coin.h"
#include "actor/Door.h"
#include "actor/Goal.h"
#include "actor/Laser.h"
#include "actor/LaserEmitter.h"
#include "actor/Physbox.h"
#include "actor/TestActor.h"

void RegisterGameActors()
{
	RegisterCoin();
	RegisterDoor();
	RegisterGoal();
	RegisterLaser();
	RegisterLaserEmitter();
	RegisterPhysbox();
	RegisterTestActor();
}
