# c-game-engine

[![Windows Build](https://github.com/droc101/c-game-engine/actions/workflows/windows.yml/badge.svg)](https://github.com/droc101/c-game-engine/actions/workflows/windows.yml)[![Linux (x86_64) Build](https://github.com/droc101/c-game-engine/actions/workflows/linux-x86_64.yml/badge.svg)](https://github.com/droc101/c-game-engine/actions/workflows/linux-x86_64.yml)[![Linux (ARM64) Build](https://github.com/droc101/c-game-engine/actions/workflows/linux-arm.yml/badge.svg)](https://github.com/droc101/c-game-engine/actions/workflows/linux-arm.yml)

Old-School FPS game (and engine) written in C using [SDL2](https://www.libsdl.org/) for platform
abstraction, [OpenGL/Vulkan](https://www.khronos.org/) for graphics, [Jolt](https://github.com/jrouwe/JoltPhysics) for
physics,
and [zlib](https://www.zlib.net/) for compression.

Runs on x86_64 Windows and Linux.

## Building
See the [wiki page](https://wiki.droc101.dev/index.php/Building_GAME) for instructions on building.

## Minimum System Requirements
### CPU

- This project aims to be compatible with all x86_64 CPUs, however, we recommend a CPU that meets x86_64-v3 (Intel
  Haswell/AMD Excavator or newer)
  - If your x86_64 CPU is not compatible, please let us know!
- There is experimental support for arm64 CPUs on Linux, however this is not intended for distribution and no support
  will be
  provided
- A CPU with 4 or more hardware threads is strongly recommended, but not required
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
