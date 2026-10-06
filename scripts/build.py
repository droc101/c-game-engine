#!/bin/python

import argparse
import os
import sys
import subprocess
import shutil
import shlex


def clean_build_directory(build_dir: str):
    """Clean files from the build directory that should not persist between builds"""
    if not os.path.isdir(build_dir):
        return

    preserved = [
        "libsteam_api.so",
        "steam_api64.dll",
        "discord_game_sdk.so",
        "discord_game_sdk.dll",
        "out",
        "_deps"
    ]

    for file in os.listdir(build_dir):
        filename = os.fsdecode(file)
        if filename in preserved:
            continue
        joined_path = os.path.join(build_dir, filename)
        if os.path.isdir(joined_path):
            shutil.rmtree(joined_path)
        else:
            os.remove(joined_path)

    deps_dir = os.path.join(build_dir, "_deps")
    for file in os.listdir(deps_dir):
        filename = os.fsdecode(file)
        if filename.startswith("sdl3") or filename.endswith("-src"):
            continue
        joined_path = os.path.join(deps_dir, filename)
        shutil.rmtree(joined_path)


def do_build(build_dir: str, build_type: str, configure_flags: list[str], build_flags: list[str]):
    """Configure and build using cmake with the specified flags"""
    configure_arguments = ["cmake", "-B", build_dir, f"-DCMAKE_BUILD_TYPE={build_type}"] + configure_flags
    configure_process = subprocess.run(configure_arguments)
    if configure_process.returncode != 0:
        print(f"Failed to configure: return code {configure_process.returncode}")
        quit()

    build_arguments = ["cmake", "--build", build_dir, "-j"] + build_flags
    build_process = subprocess.run(build_arguments)
    if build_process.returncode != 0:
        print(f"Failed to build: return code {build_process.returncode}")
        quit()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--build-dir", default="build", required=False, help="Where to place the build files")
    parser.add_argument("--build-type", required=False, default="Release",
                        choices=["Debug", "RelWithDebInfo", "Release"], help="The CMake build type to use")
    parser.add_argument("--x86-version", required=False, default="*",
                        choices=["1", "2", "3", "4", "*"], help="The x86_64 ABI level(s) to build")
    parser.add_argument("--configure-flags", required=False, default="",
                        help="Additional arguments to pass to CMake's configure stage")
    parser.add_argument("--build-flags", required=False, default="",
                        help="Additional arguments to pass to CMake's build stage")

    args = parser.parse_args(sys.argv[1:])

    if not os.path.isfile("CMakeLists.txt"):
        print("No CMakeLists.txt found, please run this from the root of the source tree.")
        quit()

    x86_versions = ["1", "2", "3", "4"]
    if args.x86_version != "*":
        x86_versions = [args.x86_version]

    clean_build_directory(args.build_dir)
    for v in x86_versions:
        print(f"---------- Building x86_64v{v} ----------")
        configure_flags = [f"-DX86_64_VERSION={v}"] + shlex.split(args.configure_flags)
        build_flags = ["--target game"] + shlex.split(args.build_flags)
        do_build(args.build_dir, args.build_type, configure_flags, build_flags)
        clean_build_directory(args.build_dir)

    print(f"----------- Building Launcher -----------")
    launcher_configure_flags = [f"-DX86_64_VERSION=1", "-DSTANDALONE_LAUNCHER=ON"] + shlex.split(args.configure_flags)
    launcher_build_flags = ["--target launcher"] + shlex.split(args.build_flags)
    do_build(args.build_dir, args.build_type, launcher_configure_flags, launcher_build_flags)
    clean_build_directory(args.build_dir)


if __name__ == "__main__":
    main()
