//
// Created by droc101 on 6/25/2024.
//

#ifndef GAME_CONFIG_H
#define GAME_CONFIG_H

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

/// Player movement speed
#define MOVE_SPEED 0.237f
/// Player movement speed when shift is held
#define SLOW_MOVE_SPEED 0.0237f

#define STICK_DEADZONE 0.1

#pragma endregion

#pragma region Debug

// Skip the splash screen
//#define DEBUG_NOSPLASH

// Enable or disable the frame graph (just the rendering)
#define FRAMEGRAPH_ENABLE
#define TICKGRAPH_ENABLE
// See /Debug/FrameGrapher.h for additional frame grapher settings

// Show debug print statements in the top left corner
#define ENABLE_DEBUG_PRINT

// Shows a level select after main menu and pause exit course.
#define USE_LEVEL_SELECT

// Show error trace in release builds (function, source, and line #)
#define ERROR_TRACE_IN_RELEASE

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

#if defined(__LINUX__) && defined(BUILDSTYLE_DEBUG)
/// Adds an overlay with FPS information provided by the Mesa Vulkan driver in Linux
// #define VK_ENABLE_MESA_FPS_OVERLAY
#endif

#pragma endregion

// Program information
#define COPYRIGHT "2025 Droc101 Development"
#define GAME_TITLE "Game" // Used for window title

#define VERSION_MAJOR 0
#define VERSION_MINOR 0
#define VERSION_PATCH 1
#define VERSION_SUFFIX "-dev"
#define VERSION TO_STR(VERSION_MAJOR) "." TO_STR(VERSION_MINOR) "." TO_STR(VERSION_PATCH) VERSION_SUFFIX

#define APPDATA_ORG_NAME "Droc101 Development"
#define APPDATA_APP_NAME "Game"

/// The level to start on when the game loads
#define STARTING_LEVEL "leveldata_test"

/// Number of sound effect channels
/// (sound effects that can play at the same time)
#define SFX_CHANNEL_COUNT 16

/// Target physics updates per second (be careful with this)
#define PHYSICS_TARGET_TPS 60

/// Field of view
#define FOV 90
/// Near clipping plane
#define NEAR_Z 0.01
/// Far clipping plane
#define FAR_Z 1000

#define MAX_HEALTH 100

#endif //GAME_CONFIG_H
