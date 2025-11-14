cmake_minimum_required(VERSION 3.18)
project(launcher C)

add_executable(launcher
        src/main.c
        src/LibraryLoader.c
        include/LibraryLoader.h
)
set_target_properties(launcher PROPERTIES OUTPUT_NAME "game")
if (x86_64)
    target_compile_options(launcher PRIVATE "-march=x86-64")
else ()
    target_compile_options(launcher PRIVATE "-march=native")
endif ()

if (WIN32)
    target_compile_options(launcher PRIVATE "-mwindows")
    target_sources(launcher PRIVATE res/game.rc)
    target_link_libraries(launcher PRIVATE comctl32)
endif ()

add_custom_target(
        copy_assets
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets
)

add_custom_target(copy_libraries ALL
        COMMENT "Copying shared libraries to build output directory"
)
if (WIN32)

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
                "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}\\${dll}"
        )
    endforeach ()
endif ()
add_dependencies(launcher copy_assets copy_libraries game)
