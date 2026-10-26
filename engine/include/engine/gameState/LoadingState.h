//
// Created by droc101 on 1/20/25.
//

/**
 * @file LoadingState.h
 * GameState that safely transitions between maps
 */

#ifndef LOADINGSTATE_H
#define LOADINGSTATE_H

#include <engine/structs/GameState.h>
#include <engine/structs/Map.h>

/// Callback signature for when a map finishes loading
typedef void (*LoadingStateDoneFunction)(void);
/// Callback signature for when a map fails to load
typedef void (*LoadingStateErrorFunction)(void);

/// Callback run when LoadingState successfully loads a map
extern LoadingStateDoneFunction LoadingStateDoneCallback;
/// Callback run when LoadingState fails to load a map
extern LoadingStateErrorFunction LoadingStateErrorCallback;

/// The map to load
extern char *loadStateLevelname;
/// The map transition to use
extern MapTransition *loadStateTransition;

/// Loading state definition
extern const GameState LoadingState;

#endif //LOADINGSTATE_H
