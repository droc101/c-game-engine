cmake_minimum_required(VERSION 3.21) # Required by Luna dependency
project(engine C CXX)

include(CheckIncludeFile)
include(cmake/FetchPackage.cmake)
include(cmake/Helpers.cmake)

detect_platform()

#region Dependencies

find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(ZLIB REQUIRED)
find_package(OpenGL REQUIRED)

fetch_glew()
fetch_dict()

message(STATUS "Fetching Luna...")
fetch_package_tag(https://github.com/NBT22/Luna "v0.2.*" Luna)
if (x86_64)
    fetch_discord_sdk()
endif ()

message(STATUS "Fetching CGLM...")
set(CGLM_SHARED OFF)
set(CGLM_STATIC ON)
fetch_package_tag(https://github.com/recp/cglm.git v0.*.* cglm)

if (JOLT_BUILD_TYPE_RELWITHDEBINFO AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(JPH_BUILD_TYPE RelWithDebInfo)
endif ()
message(STATUS "Fetching JoltC/Jolt...")
fetch_package(https://github.com/NBT22/joltc main joltc)

#endregion

add_library(engine INTERFACE)
target_sources(engine INTERFACE
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
        src/actor/GlobalLight.c
        include/engine/actor/GlobalLight.h
        src/actor/GlobalFog.c
        include/engine/actor/GlobalFog.h

        src/assets/AssetReader.c
        include/engine/assets/AssetReader.h
        src/assets/DataReader.c
        include/engine/assets/DataReader.h
        src/assets/FontLoader.c
        include/engine/assets/FontLoader.h
        src/assets/GameConfigLoader.c
        include/engine/assets/GameConfigLoader.h
        src/assets/MapLoader.c
        include/engine/assets/MapLoader.h
        src/assets/ModelLoader.c
        include/engine/assets/ModelLoader.h
        src/assets/ShaderLoader.c
        include/engine/assets/ShaderLoader.h
        src/assets/TextureLoader.c
        include/engine/assets/TextureLoader.h
        src/assets/MapMaterialLoader.c
        include/engine/assets/MapMaterialLoader.h

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
        src/graphics/gl/GLdebug.c
        src/graphics/gl/GLinit.c
        include/engine/graphics/gl/GLinit.h
        src/graphics/gl/GLobjects.c
        include/engine/graphics/gl/GLobjects.h
        src/graphics/gl/GLui.c
        include/engine/graphics/gl/GLui.h
        src/graphics/gl/GLworld.c
        include/engine/graphics/gl/GLdebug.h
        src/graphics/gl/GLframe.c
        include/engine/graphics/gl/GLframe.h
        src/graphics/gl/GLshaders.c
        include/engine/graphics/gl/GLshaders.h
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
        src/structs/Map.c
        include/engine/structs/Map.h
        src/structs/List.c
        include/engine/structs/List.h
        src/structs/Options.c
        include/engine/structs/Options.h
        src/structs/Player.c
        include/engine/structs/Player.h
        include/engine/structs/Vector2.h
        include/engine/structs/Viewmodel.h
        src/structs/ActorWall.c
        include/engine/structs/ActorWall.h

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
set_target_properties(engine PROPERTIES LINKER_LANGUAGE CXX LINK_FLAGS "-Wl,-rpath='$ORIGIN'" PREFIX "")
target_include_directories(engine INTERFACE include)
if (x86_64)
    target_compile_definitions(engine INTERFACE ENABLE_DISCORD_SDK)
    target_include_directories(engine INTERFACE ${DISCORD_GAME_SDK_DIR}/c/)
    target_link_libraries(engine INTERFACE discord_game_sdk)
endif ()

if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
    target_compile_definitions(engine INTERFACE BUILDSTYLE_RELEASE)
else ()
    target_compile_definitions(engine INTERFACE BUILDSTYLE_DEBUG)
endif ()

if (JOLT_BUILD_DEBUG_RENDERER)
    target_compile_definitions(engine INTERFACE JPH_DEBUG_RENDERER)
endif ()
if (JOLT_BUILD_ENABLE_ASSERTS)
    target_compile_definitions(engine INTERFACE JPH_ENABLE_ASSERTS)
endif ()
target_link_libraries(engine INTERFACE
        SDL2::SDL2
        SDL2_mixer
        ZLIB::ZLIB
        joltc
        cglm
        dict
        Luna
        OpenGL::GL
        glew_s
)

if (UNIX)
    target_link_libraries(engine INTERFACE m)
elseif (WIN32)
    target_link_libraries(engine INTERFACE opengl32 dwmapi)
endif ()
