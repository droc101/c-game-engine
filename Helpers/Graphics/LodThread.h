//
// Created by NBT22 on 6/13/25.
//

#ifndef LODTHREAD_H
#define LODTHREAD_H

void LodThreadInit();

void LodThreadDestroy();

int SignalLodThreadCanStart();

int WaitForLodThreadToEnd();

int LockLodThreadMutex();

int UnlockLodThreadMutex();

#endif //LODTHREAD_H
