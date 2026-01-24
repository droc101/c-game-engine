//
// Created by droc101 on 4/21/25.
//

#ifndef IOPROXY_H
#define IOPROXY_H

#include <engine/structs/ActorDefinition.h>

extern ActorDefinition ioProxyActorDefinition;

#define IO_PROXY_ACTOR_NAME "io_proxy"

#define IO_PROXY_OUTPUT_FIRST_TICK "first_tick"

void RegisterIoProxy();

#endif //IOPROXY_H
