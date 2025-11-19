cmake_minimum_required(VERSION 3.18) # TODO: Figure out the real number for all of these (and test with that version)
project(game C)

include(engine/cmake/Helpers.cmake)

#region Options

set(CMAKE_C_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out/bin) # TODO github actions spits out things into this directory because we build some of the deps
file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
file(MAKE_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

option(JOLT_BUILD_DEBUG_RENDERER "Whether or not to enable the Jolt debug renderer. Note that this has a performance impact even if the renderer is not visible." ON)
option(JOLT_BUILD_ENABLE_ASSERTS "Whether or not to enable the Jolt's assert macros. This overrides the build type and forces them to be enabled always." ON)
option(JOLT_BUILD_TYPE_RELWITHDEBINFO "Whether or not to compile Jolt using a RelWithDebInfo build even when compiling a debug build of GAME." ON)

option(STANDALONE_LAUNCHER "Whether or not to build only the launcher" OFF)
set(X86_64_VERSION "3" CACHE STRING "The x86_64 microarchitecture level to target, 1 to 4. Using lower levels has a performance penalty.") # See https://en.wikipedia.org/wiki/X86-64#Microarchitecture_levels for details

#endregion

#region Compile flags

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wno-missing-field-initializers -Wno-c23-extensions -fvisibility=hidden -Wundef")

set(CMAKE_C_FLAGS_DEBUG "-g -O0 -DBUILDSTYLE_DEBUG")
set(CMAKE_C_FLAGS_RELEASE "-g0 -O3 -s -DNDEBUG -Wl,--gc-sections -fdata-sections -ffunction-sections -DBUILDSTYLE_RELEASE")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-g -O3 -Wl,--gc-sections -fdata-sections -ffunction-sections -DBUILDSTYLE_DEBUG -fwhole-program")

# Enable LTO in release builds
if (CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    enable_lto()
endif ()

#endregion

detect_platform()

set(ENGINE_SOURCE_DIR ${CMAKE_SOURCE_DIR} CACHE PATH "The root directory of the engine, containing both the engine and the launcher projects")

if (NOT STANDALONE_LAUNCHER)
    add_subdirectory(game)
else()
    add_custom_target(game)
endif ()

add_subdirectory(${ENGINE_SOURCE_DIR}/launcher launcher)
