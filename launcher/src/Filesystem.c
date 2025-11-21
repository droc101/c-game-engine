//
// Created by droc101 on 11/14/2025.
//

#include "../include/Filesystem.h"

#ifdef WIN32
#include <io.h>
#include <minwindef.h>
#include <pathcch.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <windows.h>
#include <winerror.h>
#include <winnt.h>
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

	for (size_t i = 0; i < wcslen(directory); i++)
	{
		if (directory[i] == L'/')
		{
			directory[i] = L'\\';
		}
	}

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
