cmake_minimum_required(VERSION 3.18)
project(game C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

#region Compile options

# Enable LTO in release builds
if (CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    include(CheckIPOSupported)
    check_ipo_supported(RESULT result)
    if (result)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
        message(STATUS "IPO/LTO is supported")
    else ()
        message(WARNING "IPO/LTO is not supported")
    endif ()
endif ()

#endregion

add_subdirectory(engine)

#region Game Executable

add_executable(game
        $<$<BOOL:${WIN32}>:game/game.rc> # Add the Win32 resource file only on Windows

        game/src/actor/Coin.c
        game/include/actor/Coin.h
        game/src/actor/Door.c
        game/include/actor/Door.h
        game/src/actor/Goal.c
        game/include/actor/Goal.h
        game/src/actor/Laser.c
        game/include/actor/Laser.h
        game/src/actor/LaserEmitter.c
        game/include/actor/LaserEmitter.h
        game/src/actor/Physbox.c
        game/include/actor/Physbox.h
        game/src/actor/TestActor.c
        game/include/actor/TestActor.h

        game/src/gameState/LevelSelectState.c
        game/include/gameState/LevelSelectState.h
        game/src/gameState/LoadingState.c
        game/include/gameState/LoadingState.h
        game/src/gameState/LogoSplashState.c
        game/include/gameState/LogoSplashState.h
        game/src/gameState/MainState.c
        game/include/gameState/MainState.h
        game/src/gameState/MenuState.c
        game/include/gameState/MenuState.h
        game/src/gameState/OptionsState.c
        game/include/gameState/OptionsState.h
        game/src/gameState/options/InputOptionsState.c
        game/include/gameState/options/InputOptionsState.h
        game/src/gameState/options/SoundOptionsState.c
        game/include/gameState/options/SoundOptionsState.h
        game/src/gameState/options/VideoOptionsState.c
        game/include/gameState/options/VideoOptionsState.h
        game/src/gameState/PauseState.c
        game/include/gameState/PauseState.h

        game/src/helpers/GameActorRegistration.c
        game/include/helpers/GameActorRegistration.h

        game/src/main.c
)
target_link_libraries(game PRIVATE engine)
target_include_directories(game PRIVATE game/include)
set_target_properties(game PROPERTIES LINKER_LANGUAGE CXX LINK_FLAGS "-Wl,-rpath='$ORIGIN'")
if (x86_64)
    fetchDiscordGameSDK(engine)
    target_compile_definitions(game PRIVATE ENABLE_DISCORD_SDK)
endif ()

#endregion

#region Asset Generation

add_custom_target(
        copy_assets
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets
)

add_dependencies(game copy_assets)

#endregion

#region Copy Libraries

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

    add_dependencies(game copy_libraries)
endif ()

#endregion
