cmake_minimum_required(VERSION 3.18)
project(launcher C)

add_executable(launcher
        src/main.c
        src/LibraryLoader.c
        include/LibraryLoader.h
        src/DialogSystem.c
        include/DialogSystem.h
        include/AnsiCodes.h
        src/Filesystem.c
        include/Filesystem.h
)
set_target_properties(launcher PROPERTIES OUTPUT_NAME "game")
if (x86_64)
    target_compile_options(launcher PRIVATE "-march=x86-64")
    target_sources(launcher PRIVATE src/x86.c include/x86.h)
else ()
    target_compile_options(launcher PRIVATE "-march=native")
endif ()

if (WIN32)
    target_compile_options(launcher PRIVATE "-mwindows")
    target_sources(launcher PRIVATE res/game.rc)
    target_link_libraries(launcher PRIVATE comctl32 pathcch)
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
            COMMAND cygpath -w /ucrt64
            OUTPUT_VARIABLE MSYS_PATH
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    set(DLLS
            zlib1.dll
            libwinpthread-1.dll
            libgcc_s_seh-1.dll
            libstdc++-6.dll
    )

    foreach (DLL IN LISTS DLLS)
        add_custom_command(TARGET copy_libraries POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${MSYS_PATH}\\bin\\${DLL}"
                "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}\\${DLL}"
        )
    endforeach ()

    if (x86_64)
        add_custom_command(TARGET copy_libraries POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_BINARY_DIR}/discord_game_sdk.dll"
                "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/discord_game_sdk.dll"
        )
    endif ()
elseif (x86_64) # linux x86
    add_custom_command(TARGET copy_libraries POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_BINARY_DIR}/discord_game_sdk.so"
            "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/discord_game_sdk.so"
    )
endif ()
add_dependencies(launcher copy_assets copy_libraries game)
