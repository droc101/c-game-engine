# This script copies the necessary DLLs to the build directory after building the project.

import shutil
import sys
import os

if len(sys.argv) != 2:
	print("Usage: postbuild-windows.py <path-to-build-dir>")
	sys.exit(1)

build_dir = sys.argv[1]
mingw_path = os.popen("cygpath -w /mingw64").read().strip()

shutil.copy("lib/SDL2/x86_64-w64-mingw32/bin/SDL2.dll", build_dir + "/SDL2.dll")
shutil.copy("lib/SDL2_mixer/x86_64-w64-mingw32/bin/SDL2_mixer.dll", build_dir + "/SDL2_mixer.dll")
shutil.copy(mingw_path + "\\bin\\zlib1.dll", build_dir + "/zlib1.dll")