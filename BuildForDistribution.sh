#!/bin/bash
set -eo pipefail

shopt -s globstar
shopt -s extglob

SRC_DIR=$PWD
BUILD_DIR=$1
if [ -z "$BUILD_DIR" ]; then
  BUILD_DIR=$PWD/build
fi

export CMAKE_BUILD_PARALLEL_LEVEL=$(nproc)

clean_build_dir() {
  if [ -d "$BUILD_DIR" ]; then
    cd "$BUILD_DIR" # Make sure we don't break things by being in the wrong directory :)
    rm -rf !(out|_deps) _deps/!(*-src)/
  fi
  cd "$SRC_DIR"
}

for i in {1..4}; do
  clean_build_dir
  echo "---------- CONFIGURING x86_64 v$i ----------"
  cmake -B "$BUILD_DIR" -DX86_64_VERSION=$i -DCMAKE_BUILD_TYPE=$2 $3
  echo "---------- BUILDING x86_64 v$i ----------"
  cmake --build "$BUILD_DIR" --target game $4
done

# MSYSTEM is defined by MSYS2
# TODO should we check if it's equal to "MINGW64"?
if [ -z "$MSYSTEM" ]; then
  cp "$BUILD_DIR/discord_game_sdk.so" "$BUILD_DIR/out/bin"
else
  cp "$BUILD_DIR/discord_game_sdk.dll" "$BUILD_DIR/out/bin"
fi
clean_build_dir

echo "---------- CONFIGURING Launcher ----------"
cmake -B "$BUILD_DIR" -DX86_64_VERSION=$i -DSTANDALONE_LAUNCHER=ON -DCMAKE_BUILD_TYPE=$2 $3
echo "---------- BUILDING Launcher ----------"
cmake --build "$BUILD_DIR" --target launcher $4
clean_build_dir
