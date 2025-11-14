//
// Created by droc101 on 11/14/2025.
//

#include "../include/Filesystem.h"

#ifdef WIN32
#include <io.h>
#include <pathcch.h>
#include <stdio.h>
#include <windows.h>
#else
#include <libgen.h>
#include <unistd.h>
#endif

char *GetDirectoryOfFile(char *fileName)
{
#ifdef WIN32
	wchar_t *directory = malloc(sizeof(wchar_t) * MAX_PATH);
	if (!directory)
	{
		return NULL;
	}
	mbstowcs(directory, fileName, MAX_PATH);
	const HRESULT h = PathCchRemoveFileSpec(directory, MAX_PATH);
	if (!SUCCEEDED(h))
	{
		free(directory);
		return NULL;
	}
	char *outBuf = malloc(sizeof(char) * MAX_PATH);
	if (!outBuf)
	{
		free(directory);
		return NULL;
	}
	wcstombs(outBuf, directory, MAX_PATH);
	free(directory);
	return outBuf;
#else
	char *directory = dirname(fileName);
	return directory;
#endif
}

void ChangeDirectory(const char *dirName)
{
	chdir(dirName);
}
