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
find_package(GLEW REQUIRED)
message("fetch dict")
fetch_dict()
message("fetch luna")
fetch_package_tag(https://github.com/NBT22/Luna v0.2.* Luna)
if (x86_64)
    message("fetch discord sdk")
    fetch_discord_sdk()
endif ()
check_include_file("cglm/cglm.h" cglm_FOUND)
if (NOT cglm_FOUND)
    fetch_package_tag(https://github.com/recp/cglm.git v0.*.* cglm)
else ()
    add_library(cglm INTERFACE)
endif ()

if (JOLT_BUILD_TYPE_RELWITHDEBINFO AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(JPH_BUILD_TYPE RelWithDebInfo)
endif ()
message("fetch jolt")
fetch_package(https://github.com/NBT22/joltc main joltc)
set_target_properties(Jolt PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(joltc PROPERTIES POSITION_INDEPENDENT_CODE ON)

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
target_include_directories(engine INTERFACE include)
if (x86_64)
    target_compile_definitions(engine INTERFACE ENABLE_DISCORD_SDK)
    target_include_directories(engine INTERFACE ${DISCORD_GAME_SDK_DIR}/c/)
    target_link_libraries(engine INTERFACE discord_game_sdk)
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
        GLEW::GLEW
)

if (UNIX)
    target_link_libraries(engine INTERFACE m)
elseif (WIN32)
    target_link_libraries(engine INTERFACE opengl32 dwmapi)
endif ()
