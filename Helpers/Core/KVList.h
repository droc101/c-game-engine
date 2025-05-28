//
// Created by droc101 on 5/28/25.
//

#ifndef KVLIST_H
#define KVLIST_H
#include "../../defines.h"
#include "List.h"

typedef struct KvList KvList;

struct KvList
{
	List keys;
	List values;
};

void KvListCreate(KvList *list);

void KvSet(KvList *list, const char *key, Param value);

Param *KvGet(const KvList *list, const char *key);

void KvDelete(KvList *list, const char *key);

void KvListDestroy(KvList *list);

#endif //KVLIST_H
