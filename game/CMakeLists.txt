include(cmake/Helpers.cmake)

#region Game Executable

add_custom_target(
        copy_assets
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets
)

if (NOT WIN32)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -rdynamic")
endif ()

set(SOURCES
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
    disable_options(USE_SSE4_1 USE_SSE4_2 USE_AVX USE_AVX2 USE_AVX512 USE_LZCNT USE_TZCNT USE_F16C USE_FMADD)
    function(create_x86_executable version march)
        create_executable(game_x86_${version} ${march} "${SOURCES}" ${ARGN})
        set_target_properties(game_x86_${version} PROPERTIES OUTPUT_NAME "game.x86${version}")
    endfunction()

    create_x86_executable(v1 x86-64)
    create_x86_executable(v2 x86-64-v2 JPH_USE_SSE4_1 JPH_USE_SSE4_2)
    create_x86_executable(v3 x86-64-v3 JPH_USE_SSE4_1 JPH_USE_SSE4_2 JPH_USE_AVX JPH_USE_AVX2 JPH_USE_LZCNT JPH_USE_TZCNT JPH_USE_F16C JPH_USE_FMADD)
    create_x86_executable(v4 x86-64-v4 JPH_USE_SSE4_1 JPH_USE_SSE4_2 JPH_USE_AVX JPH_USE_AVX2 JPH_USE_LZCNT JPH_USE_TZCNT JPH_USE_F16C JPH_USE_FMADD JPH_USE_AVX512)
else ()
    create_executable(game_arm64 native "${SOURCES}") # Setting march to native because we do not intend to distribute arm64
    set_target_properties(game_arm64 PROPERTIES OUTPUT_NAME "game.arm64")
endif ()

#endregion
