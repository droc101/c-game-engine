cmake_minimum_required(VERSION 3.25)
project(game C CXX)

#region Compile options

set(CMAKE_C_STANDARD 11)

set(CMAKE_C_FLAGS "-Wall -Wextra -Wno-missing-field-initializers -Wno-c23-extensions -fvisibility=hidden")

set(CMAKE_C_FLAGS_DEBUG "-g -O0 -DBUILDSTYLE_DEBUG")
set(CMAKE_C_FLAGS_RELEASE "-g0 -O3 -s -DNDEBUG -Wl,--gc-sections -fdata-sections -ffunction-sections -DBUILDSTYLE_RELEASE")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-g -O3 -Wl,--gc-sections -fdata-sections -ffunction-sections -DBUILDSTYLE_DEBUG -fwhole-program")

if (WIN32)
    set(CMAKE_AUTO_RCC ON)
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -mwindows") # Hides console window
else ()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -rdynamic")
endif ()

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

if (CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64|ARM64|arm64)")
    message(STATUS "Detected ARM64 architecture (no support will be provided)")
    set(TARGET_ARCH "arm64")
elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64)")
    message(STATUS "Detected x86_64 architecture")
    set(TARGET_ARCH "x86_64")
else ()
    message(FATAL_ERROR "Unsupported architecture: ${CMAKE_SYSTEM_PROCESSOR}. Supported architectures are x86_64 and aarch64.")
endif ()

#endregion

#region Game Executable

add_executable(
        game
        main.c
        Helpers/Core/Input.h
        Helpers/Core/Input.c
        defines.h
        Helpers/Graphics/Drawing.h
        Helpers/Graphics/Drawing.c
        Structs/Vector2.h
        Helpers/Core/MathEx.h
        Helpers/Core/MathEx.c
        Structs/Wall.h
        Structs/Wall.c
        Helpers/Core/List.h
        Helpers/Core/List.c
        Structs/Level.h
        Structs/Level.c
        Helpers/Graphics/Font.c
        Helpers/Graphics/Font.h
        Helpers/Core/Error.c
        Helpers/Core/Error.h
        Helpers/Core/AssetLoaders/LevelLoader.c
        Helpers/Core/AssetLoaders/LevelLoader.h
        Structs/GlobalState.c
        Structs/GlobalState.h
        GameStates/GMainState.c
        GameStates/GMainState.h
        GameStates/GMenuState.c
        GameStates/GMenuState.h
        Structs/Actor.c
        Structs/Actor.h
        Actor/TestActor.c
        Actor/TestActor.h
        GameStates/GPauseState.c
        GameStates/GPauseState.h
        Debug/FrameGrapher.c
        Debug/FrameGrapher.h
        Debug/DPrint.c
        Debug/DPrint.h
        Helpers/Core/Timing.c
        Helpers/Core/Timing.h
        Helpers/Core/AssetReader.c
        Helpers/Core/AssetReader.h
        Helpers/Core/DataReader.c
        Helpers/Core/DataReader.h
        config.h
        GameStates/GLevelSelectState.c
        GameStates/GLevelSelectState.h
        Helpers/CommonAssets.c
        Helpers/CommonAssets.h
        Actor/Coin.c
        Actor/Coin.h
        Actor/Goal.c
        Actor/Goal.h
        GameStates/GLogoSplashState.c
        GameStates/GLogoSplashState.h
        Helpers/Graphics/GL/GLHelper.c
        Helpers/Graphics/GL/GLHelper.h
        Helpers/Graphics/RenderingHelpers.c
        Helpers/Graphics/RenderingHelpers.h
        Helpers/Graphics/GL/GLDebug.c
        Structs/UI/UiStack.c
        Structs/UI/UiStack.h
        Structs/UI/Controls/Button.c
        Structs/UI/Controls/Button.h
        Structs/UI/Controls/Slider.c
        Structs/UI/Controls/Slider.h
        $<$<BOOL:${WIN32}>:game.rc> # Add the Win32 resource file only on Windows
        Structs/Options.c
        Structs/Options.h
        GameStates/GOptionsState.c
        GameStates/GOptionsState.h
        Structs/UI/Controls/CheckBox.c
        Structs/UI/Controls/CheckBox.h
        Structs/UI/Controls/RadioButton.c
        Structs/UI/Controls/RadioButton.h
        Helpers/Core/Logging.c
        Helpers/Core/Logging.h
        Actor/Door.c
        Actor/Door.h
        Helpers/PlatformHelpers.c
        Helpers/PlatformHelpers.h
        Helpers/Graphics/GL/GLInternal.h
        GameStates/Options/GVideoOptionsState.c
        GameStates/Options/GVideoOptionsState.h
        GameStates/Options/GSoundOptionsState.c
        GameStates/Options/GSoundOptionsState.h
        GameStates/Options/GInputOptionsState.c
        GameStates/Options/GInputOptionsState.h
        Helpers/Core/Physics/PhysicsThread.c
        Helpers/Core/Physics/PhysicsThread.h
        Debug/FrameBenchmark.c
        Debug/FrameBenchmark.h
        Structs/UI/Controls/TextBox.c
        Structs/UI/Controls/TextBox.h
        Helpers/CommandParser.c
        Helpers/CommandParser.h
        GameStates/GLoadingState.c
        GameStates/GLoadingState.h
        Helpers/Graphics/Vulkan/Vulkan.c
        Helpers/Graphics/Vulkan/Vulkan.h
        Helpers/Graphics/Vulkan/VulkanInternal.c
        Helpers/Graphics/Vulkan/VulkanInternal.h
        Helpers/Graphics/Vulkan/VulkanHelpers.c
        Helpers/Graphics/Vulkan/VulkanHelpers.h
        Helpers/Graphics/Vulkan/VulkanResources.c
        Helpers/Graphics/Vulkan/VulkanResources.h
        Helpers/Graphics/Vulkan/VulkanActors.c
        Helpers/Graphics/Vulkan/VulkanActors.h
        Helpers/Graphics/Vulkan/VulkanPipelines.c
        Helpers/Core/Physics/Navigation.c
        Helpers/Core/Physics/Navigation.h
        Helpers/Core/Physics/Physics.c
        Helpers/Core/Physics/Physics.h
        Helpers/TextInputSystem.c
        Helpers/TextInputSystem.h
        Actor/Core/Trigger.c
        Actor/Core/Trigger.h
        Actor/Core/IoProxy.c
        Actor/Core/IoProxy.h
        Actor/Physbox.c
        Actor/Physbox.h
        Actor/Laser.c
        Actor/Laser.h
        Helpers/Core/KVList.c
        Helpers/Core/KVList.h
        Actor/Core/StaticModel.c
        Actor/Core/StaticModel.h
        Actor/Core/SoundPlayer.c
        Actor/Core/SoundPlayer.h
        Actor/Core/Sprite.c
        Actor/Core/Sprite.h
        Actor/LaserEmitter.c
        Actor/LaserEmitter.h
        Helpers/Graphics/LodThread.c
        Helpers/Graphics/LodThread.h
        Actor/Core/LogicBinary.c
        Actor/Core/LogicBinary.h
        Helpers/Core/Arguments.c
        Helpers/Core/Arguments.h
        Actor/Core/LogicDecimal.c
        Actor/Core/LogicDecimal.h
        Actor/Core/LogicCounter.c
        Actor/Core/LogicCounter.h
        Helpers/Core/AssetLoaders/TextureLoader.c
        Helpers/Core/AssetLoaders/TextureLoader.h
        Helpers/Core/AssetLoaders/ModelLoader.c
        Helpers/Core/AssetLoaders/ModelLoader.h
        Helpers/Core/AssetLoaders/FontLoader.c
        Helpers/Core/AssetLoaders/FontLoader.h
        Helpers/Core/AssetLoaders/ShaderLoader.c
        Helpers/Core/AssetLoaders/ShaderLoader.h
        Helpers/Core/SoundSystem.c
        Helpers/Core/SoundSystem.h
        Debug/JoltDebugRenderer.c
        Debug/JoltDebugRenderer.h
        Helpers/Core/Physics/Player.c
        Helpers/Core/Physics/Player.h
)

#endregion

#region Libraries

include(FetchPackage.cmake)

find_package(SDL2 REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(ZLIB REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
fetchPackage(https://github.com/NBT22/Luna v0.1.* Luna)

#add_compile_definitions(JPH_DEBUG_RENDERER) # Enables the Jolt debug renderer
FetchContent_Declare(
        joltc
        GIT_REPOSITORY https://github.com/NBT22/joltc
        GIT_TAG main
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
        EXCLUDE_FROM_ALL
        SYSTEM
)
FetchContent_MakeAvailable(joltc)

set(CROSS_PLATFORM_LIBRARIES
        ${SDL2_LIBRARIES}
        ${ZLIB_LIBRARIES}
        SDL2_mixer::SDL2_mixer
        Luna
        ${OPENGL_LIBRARIES}
        ${GLEW_LIBRARIES}
        joltc
)

if (UNIX)
    target_link_libraries(game PRIVATE ${CROSS_PLATFORM_LIBRARIES} m)
elseif (WIN32)
    target_link_libraries(game PRIVATE ${CROSS_PLATFORM_LIBRARIES} opengl32 dwmapi)
endif ()

set_target_properties(game PROPERTIES LINKER_LANGUAGE CXX)
include_directories(game PRIVATE
        ${SDL2_INCLUDE_DIRS}
        ${ZLIB_INCLUDE_DIRS}
        ${OPENGL_INCLUDE_DIR}
        ${GLEW_INCLUDE_DIRS}
)

#endregion

#region Asset Generation

add_custom_target(
        copy_assets
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/assets
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/Assets ${CMAKE_BINARY_DIR}/assets
)

add_dependencies(game copy_assets)

#endregion

#region Windows DLLs
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

    add_custom_target(copydlls ALL
            COMMENT "Copying DLLs to build output directory"
    )

    foreach (dll IN LISTS DLLS)
        add_custom_command(TARGET copydlls POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${MINGW_PATH}\\bin\\${dll}"
                "${CMAKE_BINARY_DIR}\\${dll}"
        )
    endforeach ()

    add_dependencies(game copydlls)

endif ()
#endregion
