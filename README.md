# c-game-engine
[![Linux Build](https://github.com/droc101/c-game-engine/actions/workflows/linux.yml/badge.svg)](https://github.com/droc101/c-game-engine/actions/workflows/linux.yml)[![Windows Build](https://github.com/droc101/c-game-engine/actions/workflows/windows.yml/badge.svg)](https://github.com/droc101/c-game-engine/actions/workflows/windows.yml)

Old-School FPS game (and engine) written in C using [SDL2](https://www.libsdl.org/) for platform
abstraction, [OpenGL/Vulkan](https://www.khronos.org/) for graphics, [Jolt](https://github.com/jrouwe/JoltPhysics) for
physics,
and [zlib](https://www.zlib.net/) for compression.

Runs on x86_64 Windows and Linux.

## Building
See the [wiki page](https://wiki.droc101.dev/index.php/Building_GAME) for instructions on building.

## Minimum System Requirements
### CPU
- x86_64/amd64 with the MMX, SSE, and SSE2 extensions
    - Any Intel Core I series CPU will work
    - Any AMD Ryzen CPU will work
    - Still working on determining more exact ranges, but most x86_64 CPUs will work.
- Experimental support for arm64 CPUs
- A CPU with 4 or more cores is strongly recommended, but not required.
### GPU
- Primary Vulkan 1.2 Renderer:
    - NVIDIA GeForce 900 series or newer
    - AMD Radeon HD 7000 series / Southern Islands / GCN 1.0 or newer
    - Intel HD Graphics 510 or newer
- OpenGL 3.3 Compatibility Renderer:
    - NVIDIA GeForce 8 series or newer
    - ATI Radeon HD 2000 series or newer
    - Intel HD Graphics 4000 or newer

## Tested on
- Windows 11
- Arch Linux
- Ubuntu
