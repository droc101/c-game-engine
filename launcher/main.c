//
// Created by droc101 on 11/8/25.
//

#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>

#define LIB_PREFIX "./" // TODO don't assume work dir = binary dir

#ifdef WIN32
#define LIB_SUFFIX ".dll"
#else
#define LIB_SUFFIX ".so"
#endif

typedef int (*GameMainFunction)(int argc, const char *argv[]);

int main(const int argc, const char *argv[])
{
	const char *library_basename = NULL;
#ifdef __x86_64__
	__builtin_cpu_init();
	if (__builtin_cpu_supports("x86-64-v4"))
	{
		library_basename = LIB_PREFIX "game.x86v4" LIB_SUFFIX;
	} else if (__builtin_cpu_supports("x86-64-v3"))
	{
		library_basename = LIB_PREFIX "game.x86v3" LIB_SUFFIX;
	} else if (__builtin_cpu_supports("x86-64-v2"))
	{
		library_basename = LIB_PREFIX "game.x86v2" LIB_SUFFIX;
	} else if (__builtin_cpu_supports("x86-64"))
	{
		library_basename = LIB_PREFIX "game.x86v1" LIB_SUFFIX;
	}
#else
	library_basename = LIB_PREFIX "game.arm64" LIB_SUFFIX;
#endif

	if (!library_basename)
	{
		printf("launcher: failed to find a compatible game library. exiting.\n");
		return -1;
	}

	printf("launcher: using game library \"%s\"\n", library_basename);
	void *library = dlopen(library_basename, RTLD_NOW);
	if (library)
	{
		const GameMainFunction GameMain = dlsym(library, "GameMain");
		if (GameMain)
		{
			const int ret = GameMain(argc, argv);
			dlclose(library);
			return ret;
		}
	}
	printf("launcher: error loading game library: %s\n", dlerror());
	return -1;
}
