//
// Created by droc101 on 10/26/26.
//

#include "../include/Lock.h"
#include <stdbool.h>

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
#ifdef WIN32
	singleInstanceMutex = CreateMutexW(NULL, TRUE, L"Global\\GameSingleInstanceMutex");
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		return false;
	}
	return true;
#else
	lockfile = open(LOCK_FILE_NAME, O_CREAT | O_RDWR, 0666);
	return flock(lockfile, LOCK_EX | LOCK_NB) == 0;
#endif
}

void DestroyMultiInstanceLock()
{
#ifdef WIN32
	if (singleInstanceMutex)
	{
		CloseHandle(singleInstanceMutex);
	}
#else
	if (lockfile)
	{
		flock(lockfile, LOCK_UN);
		close(lockfile);
		unlink(LOCK_FILE_NAME);
	}
#endif
}
