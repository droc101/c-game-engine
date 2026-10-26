//
// Created by droc101 on 10/26/26.
//

#include "../include/Lock.h"
#include <stdbool.h>
#include <stdlib.h>

#ifdef WIN32
#include <windows.h>

static HANDLE singleInstanceMutex = NULL;
#else
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>

#define LOCK_FILE_NAME ".lock"
static int lockfile = 0;
#endif

bool CreateMultiInstanceLock()
{
	bool lock = false;
#ifdef WIN32
	singleInstanceMutex = CreateMutexW(NULL, TRUE, L"Global\\GameSingleInstanceMutex");
	lock = GetLastError() != ERROR_ALREADY_EXISTS
#else
	lockfile = open(LOCK_FILE_NAME, O_CREAT | O_RDWR, 0666);
	lock = flock(lockfile, LOCK_EX | LOCK_NB) == 0;
#endif
		   if (lock)
	{
		atexit(DestroyMultiInstanceLock);
	}
	return lock;
}

void DestroyMultiInstanceLock()
{
#ifdef WIN32
	if (singleInstanceMutex)
	{
		CloseHandle(singleInstanceMutex);
		singleInstanceMutex = NULL;
	}
#else
	if (lockfile)
	{
		flock(lockfile, LOCK_UN);
		close(lockfile);
		lockfile = 0;
		unlink(LOCK_FILE_NAME);
	}
#endif
}
