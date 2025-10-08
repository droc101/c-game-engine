#include <stdint.h>
#include "Helpers/Core/Engine.h"
#include "Helpers/Core/Logging.h"

#undef main // Leaked by SDL_main.h

int main(const int argc, const char *argv[])
{
	InitEngine(argc, argv);
	LogInfo("Engine initialized, entering mainloop\n");
	while (!EngineShouldQuit())
	{
		EngineIteration();
	}
	LogInfo("Mainloop exited, cleaning up engine...\n");
	DestroyEngine();
	return 0;
}

#ifdef WIN32
/// Make this symbol exported (in the symbol table)
#define EXPORT_SYM __declspec(dllexport)
#else
/// Make this symbol exported (in the symbol table)
#define EXPORT_SYM __attribute__((visibility("default")))
#endif

// Exporting these symbols tells GPU drivers to use the dedicated GPU on hybrid systems
// I do not know if these do anything on Linux, but they are here just in case.
EXPORT_SYM uint32_t NvOptimusEnablement = 0x00000001;
EXPORT_SYM int AmdPowerXpressRequestHighPerformance = 1;
