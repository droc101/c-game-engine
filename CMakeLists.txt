cmake_minimum_required(VERSION 3.18)
project(game C CXX)

include(game/cmake/Helpers.cmake)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}) # TODO github actions spits out things into this directory because we build some of the deps
file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

detect_platform()

# Enable LTO in release builds
if (CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    enable_lto()
endif ()

add_executable(launcher launcher/main.c
        launcher/LibraryLoader.c
        launcher/LibraryLoader.h)
set_target_properties(launcher PROPERTIES OUTPUT_NAME "game")
if (x86_64)
    target_compile_options(launcher PRIVATE "-march=x86-64")
else ()
    target_compile_options(launcher PRIVATE "-march=native")
endif ()

if (WIN32)
    set(CMAKE_AUTO_RCC ON)
    target_compile_options(launcher PRIVATE "-mwindows")
    target_sources(launcher PRIVATE launcher/game.rc)
    target_link_libraries(launcher PRIVATE comctl32)
endif ()

add_subdirectory(engine)
add_subdirectory(game)

add_dependencies(launcher game_x86_v1 game_x86_v2 game_x86_v3 game_x86_v4)

if (WIN32)
    add_custom_target(copy_libraries ALL
            COMMENT "Copying shared libraries to build output directory"
    )

    execute_process(
            COMMAND cygpath -w /mingw64
            OUTPUT_VARIABLE MINGW_PATH
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    set(DLLS
            SDL2.dll
            SDL2_mixer.dll
            glew32.dll
            zlib1.dll
            libmpg123-0.dll
            libFLAC.dll
            libopusfile-0.dll
            libvorbisfile-3.dll
            libwavpack-1.dll
            libxmp.dll
            libvorbis-0.dll
            libogg-0.dll
            libwinpthread-1.dll
            libopus-0.dll
            libgcc_s_seh-1.dll
            libstdc++-6.dll
    )

    foreach (dll IN LISTS DLLS)
        add_custom_command(TARGET copy_libraries POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${MINGW_PATH}\\bin\\${dll}"
                "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}\\${dll}"
        )
    endforeach ()

    add_dependencies(launcher copy_libraries)
endif ()
