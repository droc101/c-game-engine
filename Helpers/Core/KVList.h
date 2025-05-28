//
// Created by droc101 on 5/28/25.
//

#ifndef KVLIST_H
#define KVLIST_H
#include "../../defines.h"

void KvListCreate(KvList *list);

void KvSet(KvList *list, const char *key, Param value);

Param *KvGet(const KvList *list, const char *key);

void KvDelete(KvList *list, const char *key);

void KvListDestroy(KvList *list);

size_t KvListLength(const KvList *list);

bool KvListHas(const KvList *list, const char *key);

Param* KvGetTypeWithDefault(const KvList *list, const char *key, ParamType expectedType, Param *defaultValue);

#endif //KVLIST_H
