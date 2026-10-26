//
// Created by droc101 on 10/26/26.
//

#include "../include/Lock.h"
#include <stdbool.h>
#include <stdlib.h>

#ifdef WIN32
#include <windows.h>

static HANDLE lockFile = INVALID_HANDLE_VALUE;
#else
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>

static int lockfile = 0;
#endif

#define LOCK_FILE_NAME ".lock"

bool CreateMultiInstanceLock()
{
	bool lock = false;
#ifdef WIN32
	lockFile = CreateFileA(LOCK_FILE_NAME, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_HIDDEN | FILE_FLAG_DELETE_ON_CLOSE, NULL);
	lock = lockFile != INVALID_HANDLE_VALUE;
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
	if (lockFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(lockFile);
		lockFile = INVALID_HANDLE_VALUE;
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
