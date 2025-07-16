cmake_minimum_required(VERSION 3.25)
project(game C CXX)

# Adds the module folder to the CMake module path
# This folder is populated by the setup-windows.py script
if (WIN32)
    if (NOT EXISTS "${CMAKE_SOURCE_DIR}/module")
        message(FATAL_ERROR "The module folder is missing. Please run setup-windows.py to download the required dependencies.")
    endif ()

    list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/module")
endif ()

#region Compile options

set(CMAKE_C_STANDARD 11)

set(CMAKE_C_FLAGS "-Wall -Wno-discarded-qualifiers")

set(CMAKE_C_FLAGS_DEBUG "-g -O0 -DBUILDSTYLE_DEBUG")
set(CMAKE_C_FLAGS_RELEASE "-g0 -O3 -s -DNDEBUG -Wl,--gc-sections -fdata-sections -ffunction-sections -DBUILDSTYLE_RELEASE")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-g -O3 -Wl,--gc-sections -fdata-sections -ffunction-sections -DBUILDSTYLE_DEBUG -fwhole-program")

# Add --static flag when building on Windows to include libwinpthread-1.dll
if (WIN32)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --static")
    set(CMAKE_AUTO_RCC ON)
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

if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64|ARM64|arm64)")
    message(STATUS "Detected ARM64 architecture (no support will be provided)")
    set(TARGET_ARCH "arm64")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64)")
    message(STATUS "Detected x86_64 architecture")
    set(TARGET_ARCH "x86_64")
else()
    message(FATAL_ERROR "Unsupported architecture: ${CMAKE_SYSTEM_PROCESSOR}. Supported architectures are x86_64 and aarch64.")
endif()

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
        Helpers/LevelLoader.c
        Helpers/LevelLoader.h
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
        Structs/Camera.c
        Structs/Camera.h
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
        Helpers/Core/PhysicsThread.c
        Helpers/Core/PhysicsThread.h
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
        Helpers/Navigation.c
        Helpers/Navigation.h
        Helpers/Collision.c
        Helpers/Collision.h
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
        Helpers/Core/LodThread.c
        Helpers/Core/LodThread.h
        Actor/Core/Logic.c
        Actor/Core/Logic.h
        Helpers/Core/Arguments.c
        Helpers/Core/Arguments.h
)

#endregion

#region Libraries

set(SDL2_PATH "${CMAKE_SOURCE_DIR}/lib/SDL2/x86_64-w64-mingw32")
set(SDL2_MIXER_PATH "${CMAKE_SOURCE_DIR}/lib/SDL2_mixer/x86_64-w64-mingw32")

find_package(SDL2 REQUIRED) # Platform abstraction
find_package(SDL2_mixer REQUIRED) # Audio
find_package(ZLIB REQUIRED) # Decompression
find_package(OpenGL REQUIRED) # Graphics
find_package(Vulkan REQUIRED) # 3d

if (UNIX)
    find_package(GLEW REQUIRED) # OpenGL Extension Wrangler
    if (TARGET_ARCH STREQUAL "arm64")
        set(BOX2D_LIB "${CMAKE_SOURCE_DIR}/lib/box2d/libbox2d_linux_arm64.a")
    else ()
        set(BOX2D_LIB "${CMAKE_SOURCE_DIR}/lib/box2d/libbox2d_linux.a")
    endif ()
elseif (WIN32) # Thank you Windows, I hate you.
    list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/lib/glew") # OpenGL Extension Wrangler
    add_definitions(-DGLEW_STATIC)
    set(GLEW_LIBRARIES "${CMAKE_SOURCE_DIR}/lib/glew/lib/Release/x64/glew32s.lib")
    set(GLEW_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/lib/glew/include")
    set(BOX2D_LIB "${CMAKE_SOURCE_DIR}/lib/box2d/libbox2d_windows.a")
endif ()

if (CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    set(LUNA_PATH "${CMAKE_SOURCE_DIR}/lib/Luna/libLuna_release.a")
else ()
    set(LUNA_PATH "${CMAKE_SOURCE_DIR}/lib/Luna/libLuna_debug.a")
endif ()

if (UNIX)
    target_link_libraries(game PRIVATE ${SDL2_LIBRARIES} ${ZLIB_LIBRARIES} ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES} ${Vulkan_LIBRARIES} ${LUNA_PATH} ${BOX2D_LIB} SDL2_mixer m) # extra l* flags are needed on Linux
elseif (WIN32)
    target_link_libraries(game PRIVATE ${SDL2_LIBRARY} ${ZLIB_LIBRARIES} ${SDL2_MIXER_LIBRARY} ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES} ${Vulkan_LIBRARIES} ${LUNA_PATH} ${BOX2D_LIB} opengl32 dwmapi) # Don't pass extra l* flags on Windows (it isn't happy with them)
    target_include_directories(game PRIVATE ${CMAKE_SOURCE_DIR}/lib/cglm/include) # C OpenGL Mathematics Library
endif ()

set_target_properties(game PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(game PRIVATE ${SDL2_INCLUDE_DIR} ${SDL2_MIXER_INCLUDE_DIR} ${ZLIB_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIR} ${GLEW_INCLUDE_DIRS} ${Vulkan_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/lib/Luna/include ${CMAKE_SOURCE_DIR}/lib/box2d/include)

#endregion

#region Asset Generation

add_custom_target( # Python script for generating assets code
        genassets ALL
        COMMAND "python" ${CMAKE_CURRENT_SOURCE_DIR}/AssetGenerator/genassets.py ${CMAKE_CURRENT_SOURCE_DIR}/Assets/ ${CMAKE_CURRENT_BINARY_DIR}/assets/
        COMMENT "Generating assets"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Assets/
)

add_dependencies(game genassets)

#endregion

#region Windows DLLs
if (WIN32)

    add_custom_target( # Python script for generating assets code
            copydlls ALL
            COMMAND "python" ${CMAKE_CURRENT_SOURCE_DIR}/postbuild-windows.py ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Copying DLLs"
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )

    add_dependencies(game copydlls)

endif ()
#endregion
