//
// Created by droc101 on 6/25/2024.
//

#ifndef GAME_CONFIG_H
#define GAME_CONFIG_H

#define STR(x) #x
#define TO_STR(x) STR(x)

#pragma region Window Settings

// Default window size
#define DEF_WIDTH 1280
#define DEF_HEIGHT 720

// Minimum and maximum window size
#define MIN_WIDTH 960
#define MIN_HEIGHT 720

// Maximum window size
#define MAX_WIDTH 9999
#define MAX_HEIGHT 9999

#pragma endregion

#pragma region Movement

/// Player movement speed (in units per second)
#define MOVE_SPEED 6.0f
/// Player movement speed when ctrl is held
#define SLOW_MOVE_SPEED 0.6f

/// The maximum angle that the player can walk on, in degrees
#define MAX_WALKABLE_SLOPE 50.0f

#define STICK_DEADZONE 0.1

#pragma endregion

#pragma region Debug

// Enable or disable the frame graph (just the rendering)
#define FRAMEGRAPH_ENABLE
#define TICKGRAPH_ENABLE
// See /Debug/FrameGrapher.h for additional frame grapher settings

// Show debug print statements in the top left corner
#define ENABLE_DEBUG_PRINT

// Shows a level select after main menu and pause exit course.
#define USE_LEVEL_SELECT

// Enable or disable the frame benchmark system
#define BENCHMARK_SYSTEM_ENABLE

// Enable or disable recording highest and lowest frame times (has a performance impact)
// #define BENCHMARK_RECORD_HIGH_LOW_TIMES

#ifdef BUILDSTYLE_DEBUG
/**
 * Additional validation for Vulkan code
 * This will only work if the Vulkan SDK is installed on the device running the program.
 * @warning NOT FOR RELEASE BUILDS
 * @see https://docs.vulkan.org/guide/latest/validation_overview.html
 * @see https://vulkan.lunarg.com/doc/sdk/latest/windows/khronos_validation_layer.html
 */
#define VK_ENABLE_VALIDATION_LAYER
#endif

// Use wireframe for the Jolt debug renderer
// Note: This does NOT actually enable the debug renderer
#define JPH_DEBUG_RENDERER_WIREFRAME

#pragma endregion

#define ENGINE_VERSION_MAJOR 0
#define ENGINE_VERSION_MINOR 0
#define ENGINE_VERSION_PATCH 1
#define ENGINE_VERSION_SUFFIX "-assets-v2"
#define ENGINE_VERSION \
	TO_STR(ENGINE_VERSION_MAJOR) "." TO_STR(ENGINE_VERSION_MINOR) "." TO_STR(ENGINE_VERSION_PATCH) ENGINE_VERSION_SUFFIX

/// Target physics updates per second (be careful with this)
#define PHYSICS_TARGET_TPS 60
/// Minimum physics updates per second. Delta time gets clamped to this valued.
#define PHYSICS_MIN_TPS 10

/// Field of view
#define FOV 90
/// Field of view used for viewmodels
#define VIEWMODEL_FOV 70
/// Near clipping plane
#define NEAR_Z 0.01
/// Far clipping plane
#define FAR_Z 1000

#define MAX_HEALTH 100

#endif //GAME_CONFIG_H
