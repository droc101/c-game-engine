//
// Created by NBT22 on 6/13/25.
//

#ifndef LODTHREAD_H
#define LODTHREAD_H

void LodThreadInit();

void LodThreadDestroy();

void SignalLodThreadCanStart();

void WaitForLodThreadToEnd();

void LockLodThreadMutex();

void UnlockLodThreadMutex();

#endif //LODTHREAD_H
