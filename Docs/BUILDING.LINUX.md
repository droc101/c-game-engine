# Building - Linux
Congratulations! Linux is far easier to build on than Windows.

### Setup
- You will need to have `cmake`, `make`, and `gcc` installed.
- You will need development packages as outlined in the libraries section.
- You need Python 3 (as the command `python`) with PIL or Pillow installed to run the asset compression script (required to build)
- You will need to compile Luna, then copy (or link) the `include` directory as well as the compiled binaries to `libLuna_debug.a` and `libLuna_release.a` in `lib/Luna`.

### Libraries

You will need development packages for CGLM, GLEW, SDL2, SDL2_Mixer, Vulkan, and ZLIB.

Arch Packages:
[cglmᴬᵁᴿ](https://aur.archlinux.org/packages/cglm),
[glew](https://archlinux.org/packages/extra/x86_64/glew/),
[sdl2-gitᴬᵁᴿ](https://aur.archlinux.org/packages/sdl2-git),
[sdl2_mixer](https://archlinux.org/packages/extra/x86_64/sdl2_mixer/),
[vulkan-devel](https://archlinux.org/groups/x86_64/vulkan-devel/),
[zlib](https://archlinux.org/packages/core/x86_64/zlib/),
[python](https://archlinux.org/packages/core/x86_64/python/),
[python-pillow](https://archlinux.org/packages/extra/x86_64/python-pillow/)

Note: While [sdl2-compat](https://archlinux.org/packages/extra/x86_64/sdl2-compat/) mostly works, there are a few bugs, so we do recommend using [sdl2-gitᴬᵁᴿ](https://aur.archlinux.org/packages/sdl2-git) instead.

### Building
There are 3 build types available:
- `Debug` - No optimizations, debug symbols and features enabled.
- `Release` - Full optimizations, no debug symbols or features.
- `RelWithDebInfo` - Full optimizations, debug symbols and features enabled.

Open the terminal in the project directory and run the following commands to build the project:
```sh
mkdir build
cd build
cmake -B . -DCMAKE_BUILD_TYPE=[Build type] ..
cmake --build . --target game -- -j
```
The compiled executable will be `game` in the `build` directory.
