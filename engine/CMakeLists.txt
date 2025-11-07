cmake_minimum_required(VERSION 3.25)
project(engine C CXX)

set(CMAKE_C_STANDARD 11)

include(cmake/FetchPackage.cmake)
include(CheckCSourceCompiles)
include(CheckIncludeFile)

add_library(engine STATIC)

#region Compile Options

option(JOLT_BUILD_DEBUG_RENDERER "Whether or not to enable the Jolt debug renderer. Note that this has a performance impact even if the renderer is not visible." OFF)
option(JOLT_BUILD_ENABLE_ASSERTS "Whether or not to enable the Jolt's assert macros. This overrides the build type and forces them to be enabled always." ON)
option(JOLT_BUILD_TYPE_RELWITHDEBINFO "Whether or not to compile Jolt using a RelWithDebInfo build even when compiling a debug build of GAME." ON)

set(X86_64_VERSION "3" CACHE STRING "The x86_64 microarchitecture level to target, 1 to 4. Using lower levels has a performance penalty.") # See https://en.wikipedia.org/wiki/X86-64#Microarchitecture_levels for details

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wno-missing-field-initializers -Wno-c23-extensions -fvisibility=hidden -Wundef")

set(CMAKE_C_FLAGS_DEBUG "-g -O0 -DBUILDSTYLE_DEBUG")
set(CMAKE_C_FLAGS_RELEASE "-g0 -O3 -s -DNDEBUG -Wl,--gc-sections -fdata-sections -ffunction-sections -DBUILDSTYLE_RELEASE")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-g -O3 -Wl,--gc-sections -fdata-sections -ffunction-sections -DBUILDSTYLE_DEBUG -fwhole-program")

if (WIN32)
    set(CMAKE_AUTO_RCC ON)
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -mwindows") # Hides console window
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -mwindows") # Hides console window
else ()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -rdynamic")
endif ()

set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
check_c_source_compiles("
    #if !((defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64) || defined(_M_X64) || defined(_M_AMD64)) && !defined(_M_ARM64EC))
        #error
    #endif" x86_64)
if (x86_64)
    message(STATUS "Detected x86_64 architecture")
    message(STATUS "Using x86_64-v${X86_64_VERSION}")
    if (X86_64_VERSION EQUAL "4")
        add_compile_definitions(X86_V4 X86_V3 X86_V2 X86_V1)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=x86-64-v4")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=x86-64-v4")
        set(USE_SSE4_1 ON)
        set(USE_SSE4_2 ON)
        set(USE_AVX ON)
        set(USE_AVX2 ON)
        set(USE_AVX512 ON)
        set(USE_LZCNT ON)
        set(USE_TZCNT ON)
        set(USE_F16C ON)
        set(USE_FMADD ON)
    elseif (X86_64_VERSION EQUAL "3")
        add_compile_definitions(X86_V3 X86_V2 X86_V1)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=x86-64-v3")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=x86-64-v3")
        set(USE_SSE4_1 ON)
        set(USE_SSE4_2 ON)
        set(USE_AVX ON)
        set(USE_AVX2 ON)
        set(USE_AVX512 OFF)
        set(USE_LZCNT ON)
        set(USE_TZCNT ON)
        set(USE_F16C ON)
        set(USE_FMADD ON)
    elseif (X86_64_VERSION EQUAL "2")
        add_compile_definitions(X86_V2 X86_V1)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=x86-64-v2")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=x86-64-v2")
        set(USE_SSE4_1 ON)
        set(USE_SSE4_2 ON)
        set(USE_AVX OFF)
        set(USE_AVX2 OFF)
        set(USE_AVX512 OFF)
        set(USE_LZCNT OFF)
        set(USE_TZCNT OFF)
        set(USE_F16C OFF)
        set(USE_FMADD OFF)
    else () # v1 and everything else
        add_compile_definitions(X86_V1)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=x86-64")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=x86-64")
        set(USE_SSE4_1 OFF)
        set(USE_SSE4_2 OFF)
        set(USE_AVX OFF)
        set(USE_AVX2 OFF)
        set(USE_AVX512 OFF)
        set(USE_LZCNT OFF)
        set(USE_TZCNT OFF)
        set(USE_F16C OFF)
        set(USE_FMADD OFF)
    endif ()
else ()
    check_c_source_compiles("
        #if !(defined(__aarch64__) || defined(_M_ARM64))
            #error
        #endif" arm64)
    if (arm64)
        message(STATUS "Detected ARM64 architecture (no support will be provided)")
    else ()
        message(FATAL_ERROR "Unsupported architecture: ${CMAKE_SYSTEM_PROCESSOR}. Supported architectures are x86_64 and aarch64.")
    endif ()
endif ()

if (NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "GAME only supports 64-bit systems (CMAKE_SIZEOF_VOID_P=${CMAKE_SIZEOF_VOID_P})")
else ()
    message(STATUS "CMAKE_SIZEOF_VOID_P=${CMAKE_SIZEOF_VOID_P}")
endif ()

#endregion Compile Options

#region Dependencies

find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(ZLIB REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
fetchDict()
fetchPackage(https://github.com/NBT22/Luna v0.2.* Luna)
if (x86_64)
    fetchDiscordGameSDK(engine)
    target_compile_definitions(engine PRIVATE ENABLE_DISCORD_SDK)
endif ()
check_include_file("cglm/cglm.h" cglm_FOUND)
if (NOT cglm_FOUND)
    fetchPackage(https://github.com/recp/cglm.git v0.*.* cglm)
else ()
    add_library(cglm INTERFACE)
endif ()

if (JOLT_BUILD_DEBUG_RENDERER)
    add_compile_definitions(JPH_DEBUG_RENDERER)
endif ()
if (JOLT_BUILD_ENABLE_ASSERTS)
    add_compile_definitions(JPH_ENABLE_ASSERTS)
endif ()
if (JOLT_BUILD_TYPE_RELWITHDEBINFO AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(JPH_BUILD_TYPE RelWithDebInfo)
endif ()
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

#endregion

target_sources(engine PRIVATE
        src/actor/IoProxy.c
        include/engine/actor/IoProxy.h
        src/actor/LogicBinary.c
        include/engine/actor/LogicBinary.h
        src/actor/LogicCounter.c
        include/engine/actor/LogicCounter.h
        src/actor/LogicDecimal.c
        include/engine/actor/LogicDecimal.h
        src/actor/SoundPlayer.c
        include/engine/actor/SoundPlayer.h
        src/actor/Sprite.c
        include/engine/actor/Sprite.h
        src/actor/StaticModel.c
        include/engine/actor/StaticModel.h
        src/actor/Trigger.c
        include/engine/actor/Trigger.h

        src/assets/AssetReader.c
        include/engine/assets/AssetReader.h
        src/assets/DataReader.c
        include/engine/assets/DataReader.h
        src/assets/FontLoader.c
        include/engine/assets/FontLoader.h
        src/assets/GameConfigLoader.c
        include/engine/assets/GameConfigLoader.h
        src/assets/LevelLoader.c
        include/engine/assets/LevelLoader.h
        src/assets/ModelLoader.c
        include/engine/assets/ModelLoader.h
        src/assets/ShaderLoader.c
        include/engine/assets/ShaderLoader.h
        src/assets/TextureLoader.c
        include/engine/assets/TextureLoader.h

        src/debug/DPrint.c
        include/engine/debug/DPrint.h
        src/debug/FrameBenchmark.c
        include/engine/debug/FrameBenchmark.h
        src/debug/FrameGrapher.c
        include/engine/debug/FrameGrapher.h
        src/debug/JoltDebugRenderer.c
        include/engine/debug/JoltDebugRenderer.h

        src/graphics/Drawing.c
        include/engine/graphics/Drawing.h
        src/graphics/Font.c
        include/engine/graphics/Font.h
        src/graphics/gl/GLDebug.c
        src/graphics/gl/GLHelper.c
        include/engine/graphics/gl/GLHelper.h
        include/engine/graphics/gl/GLInternal.h
        src/graphics/RenderingHelpers.c
        include/engine/graphics/RenderingHelpers.h
        src/graphics/vulkan/Vulkan.c
        include/engine/graphics/vulkan/Vulkan.h
        src/graphics/vulkan/VulkanActors.c
        include/engine/graphics/vulkan/VulkanActors.h
        src/graphics/vulkan/VulkanHelpers.c
        include/engine/graphics/vulkan/VulkanHelpers.h
        src/graphics/vulkan/VulkanInternal.c
        include/engine/graphics/vulkan/VulkanInternal.h
        src/graphics/vulkan/VulkanPipelines.c
        src/graphics/vulkan/VulkanResources.c
        include/engine/graphics/vulkan/VulkanResources.h

        src/helpers/Arguments.c
        include/engine/helpers/Arguments.h
        src/helpers/MathEx.c
        include/engine/helpers/MathEx.h
        src/helpers/PlatformHelpers.c
        include/engine/helpers/PlatformHelpers.h
        src/helpers/Realloc.c
        include/engine/helpers/Realloc.h

        src/physics/Navigation.c
        include/engine/physics/Navigation.h
        src/physics/Physics.c
        include/engine/physics/Physics.h
        src/physics/PlayerPhysics.c
        include/engine/physics/PlayerPhysics.h

        src/structs/Actor.c
        include/engine/structs/Actor.h
        src/structs/ActorDefinition.c
        include/engine/structs/ActorDefinition.h
        include/engine/structs/Asset.h
        include/engine/structs/Camera.h
        include/engine/structs/Color.h
        include/engine/structs/Dict.h
        src/structs/GlobalState.c
        include/engine/structs/GlobalState.h
        src/structs/KVList.c
        include/engine/structs/KVList.h
        src/structs/Level.c
        include/engine/structs/Level.h
        src/structs/List.c
        include/engine/structs/List.h
        src/structs/Options.c
        include/engine/structs/Options.h
        include/engine/structs/Param.h
        src/structs/Player.c
        include/engine/structs/Player.h
        include/engine/structs/Vector2.h
        include/engine/structs/Viewmodel.h
        src/structs/Wall.c
        include/engine/structs/Wall.h

        src/subsystem/CommandParser.c
        include/engine/subsystem/CommandParser.h
        src/subsystem/Discord.c
        include/engine/subsystem/Discord.h
        src/subsystem/Error.c
        include/engine/subsystem/Error.h
        src/subsystem/Input.c
        include/engine/subsystem/Input.h
        src/subsystem/Logging.c
        include/engine/subsystem/Logging.h
        src/subsystem/SoundSystem.c
        include/engine/subsystem/SoundSystem.h
        src/subsystem/TextInputSystem.c
        include/engine/subsystem/TextInputSystem.h
        src/subsystem/threads/LodThread.c
        include/engine/subsystem/threads/LodThread.h
        src/subsystem/threads/PhysicsThread.c
        include/engine/subsystem/threads/PhysicsThread.h
        src/subsystem/Timing.c
        include/engine/subsystem/Timing.h

        src/uiStack/controls/Button.c
        include/engine/uiStack/controls/Button.h
        src/uiStack/controls/CheckBox.c
        include/engine/uiStack/controls/CheckBox.h
        src/uiStack/controls/RadioButton.c
        include/engine/uiStack/controls/RadioButton.h
        src/uiStack/controls/Slider.c
        include/engine/uiStack/controls/Slider.h
        src/uiStack/controls/TextBox.c
        include/engine/uiStack/controls/TextBox.h
        src/uiStack/UiStack.c
        include/engine/uiStack/UiStack.h

        src/Engine.c
        include/engine/Engine.h
)
target_include_directories(engine PUBLIC include)

target_link_libraries(engine PUBLIC
        SDL2::SDL2
        joltc
        cglm
        dict
)
target_link_libraries(engine PRIVATE
        SDL2_mixer
        ZLIB::ZLIB
        Luna
        OpenGL::GL
        GLEW::GLEW
)

if (UNIX)
    target_link_libraries(engine PRIVATE m)
elseif (WIN32)
    target_link_libraries(engine PRIVATE opengl32 dwmapi)
endif ()
