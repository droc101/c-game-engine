//
// Created by droc101 on 1/20/25.
//

#ifndef LOADINGSTATE_H
#define LOADINGSTATE_H

#include <engine/structs/GameState.h>
#include <engine/structs/Map.h>

typedef void (*LoadingStateDoneFunction)(void);
typedef void (*LoadingStateErrorFunction)(void);

/// Callback run when LoadingState successfully loads a map
extern LoadingStateDoneFunction LoadingStateDoneCallback;
/// Callback run when LoadingState fails to load a map
extern LoadingStateErrorFunction LoadingStateErrorCallback;

extern char *loadStateLevelname;
extern MapTransition *loadStateTransition;

extern const GameState LoadingState;

#endif //LOADINGSTATE_H
