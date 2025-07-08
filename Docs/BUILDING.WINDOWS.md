# Building - Windows
This is the far more complicated of the two platforms to build on.

### Setup

1. Install the latest version of the Vulkan SDK. A getting started guide for the Vulkan SDK can be found [here](https://vulkan.lunarg.com/doc/sdk/latest/windows/getting_started.html).
2. Install [MSYS2 mingw64](https://www.msys2.org/)
3. Ensure `cygpath.exe` is in your `PATH` (typically installed to `C:\msys64\usr\bin`)
4. Inside MSYS2 MinGW 64-Bit, run the following commands inside the repository directory:
```sh
pacman -Syu
pacman -S mingw-w64-x86_64-toolchain mingw-w64-x86_64-python mingw-w64-x86_64-python-requests mingw-w64-x86_64-python-regex mingw-w64-x86_64-python-pillow mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-ninja mingw-w64-x86_64-shaderc mingw-w64-x86_64-vulkan-headers mingw-w64-x86_64-vulkan-loader mingw-w64-x86_64-zlib
python setup-windows.py
```
Notes:
- This project uses GCC. No other compilers are guaranteed to work.
- You may want to occasionally rerun the setup script to update the dependencies.

### CLion Setup

This setup only applies if you are using the [CLion](https://www.jetbrains.com/clion/) IDE

1. Add a new MinGW toolchain with the following settings
   - Toolchain: The path to your mingw64 install (typically `C:\msys64\mingw64`)
   - CMake: The: The `cmake.exe` from your mingw64 (typically `C:\msys64\mingw64\bin\cmake.exe`)

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
cmake --build . --target game
```
The compiled executable will be `game.exe` in the `build` directory.
If distributing, make sure to copy the DLLs as well.
