cmake_minimum_required(VERSION 3.24)
project(game_module C CXX)

#region Game Executable

add_library(game MODULE EXCLUDE_FROM_ALL
        src/actor/Coin.c
        include/actor/Coin.h
        src/actor/Door.c
        include/actor/Door.h
        src/actor/Goal.c
        include/actor/Goal.h
        src/actor/Laser.c
        include/actor/Laser.h
        src/actor/LaserEmitter.c
        include/actor/LaserEmitter.h
        src/actor/Physbox.c
        include/actor/Physbox.h
        src/actor/TestActor.c
        include/actor/TestActor.h

        src/gameState/LevelSelectState.c
        include/gameState/LevelSelectState.h
        src/gameState/LoadingState.c
        include/gameState/LoadingState.h
        src/gameState/LogoSplashState.c
        include/gameState/LogoSplashState.h
        src/gameState/MainState.c
        include/gameState/MainState.h
        src/gameState/MenuState.c
        include/gameState/MenuState.h
        src/gameState/OptionsState.c
        include/gameState/OptionsState.h
        src/gameState/options/InputOptionsState.c
        include/gameState/options/InputOptionsState.h
        src/gameState/options/SoundOptionsState.c
        include/gameState/options/SoundOptionsState.h
        src/gameState/options/VideoOptionsState.c
        include/gameState/options/VideoOptionsState.h
        src/gameState/PauseState.c
        include/gameState/PauseState.h

        src/helpers/GameActorRegistration.c
        include/helpers/GameActorRegistration.h

        src/GameMain.c
)

if (x86_64)
    if (X86_64_VERSION STREQUAL "1")
        target_compile_options(game PRIVATE -march=x86-64)
        disable_options(USE_SSE4_1 USE_SSE4_2 USE_AVX USE_AVX2 USE_AVX512 USE_LZCNT USE_TZCNT USE_F16C USE_FMADD)
    elseif (X86_64_VERSION STREQUAL "2")
        target_compile_options(game PRIVATE -march=x86-64-v2)
        disable_options(USE_AVX USE_AVX2 USE_AVX512 USE_LZCNT USE_TZCNT USE_F16C USE_FMADD)
        enable_options(USE_SSE4_1 USE_SSE4_2)
    elseif (X86_64_VERSION STREQUAL "3")
        target_compile_options(game PRIVATE -march=x86-64-v3)
        disable_options(USE_AVX512)
        enable_options(USE_SSE4_1 USE_SSE4_2 USE_AVX USE_AVX2 USE_LZCNT USE_TZCNT USE_F16C USE_FMADD)
    elseif (X86_64_VERSION STREQUAL "4")
        target_compile_options(game PRIVATE -march=x86-64-v4)
        enable_options(USE_SSE4_1 USE_SSE4_2 USE_AVX USE_AVX2 USE_AVX512 USE_LZCNT USE_TZCNT USE_F16C USE_FMADD)
    else ()
        message(FATAL_ERROR "Invalid x86_64 ABI version ${X86_64_VERSION}")
    endif ()
    set_target_properties(game PROPERTIES OUTPUT_NAME "game.x86v${X86_64_VERSION}")
else ()
    set_target_properties(game PROPERTIES OUTPUT_NAME "game.arm64")
endif ()

add_subdirectory(${ENGINE_SOURCE_DIR}/engine engine)

target_link_libraries(game PRIVATE engine)
target_include_directories(game PRIVATE include)
set_target_properties(game PROPERTIES LINKER_LANGUAGE CXX LINK_FLAGS "-Wl,-rpath='$ORIGIN'" PREFIX "")

if (x86_64)
    if (WIN32)
        # TODO: I don't like DISCORD_GAME_SDK_LIBRARY_DIR now that we're back to a more simple build structure
        add_custom_command(TARGET game PRE_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_BINARY_DIR}/discord_game_sdk.dll"
                "${CMAKE_CURRENT_BINARY_DIR}/discord_game_sdk.dll"
        )
    else ()
        add_custom_command(TARGET game PRE_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_BINARY_DIR}/discord_game_sdk.so"
                "${CMAKE_CURRENT_BINARY_DIR}/discord_game_sdk.so"
        )
    endif ()
endif ()

#endregion
