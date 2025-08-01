//
// Created by droc101 on 4/20/2024.
//

#ifndef GAME_DEFINES_H
#define GAME_DEFINES_H

#include <box2d/id.h>
#include <box2d/math_functions.h>
#include <cglm/cglm.h>
#include <SDL.h>
#include <SDL_mixer.h>
#include <stdbool.h>
#include <stdint.h>
#include "config.h"
#include "Helpers/Core/List.h"

#pragma region Forward Declarations/Typedefs

// Basic types
typedef uint8_t byte;
typedef uint16_t ushort;
typedef uint32_t uint;
typedef uint64_t ulong;

// Enum forward declarations
typedef enum CurrentState CurrentState;
typedef enum Renderer Renderer;
typedef enum OptionsMsaa OptionsMsaa;
typedef enum ModelShader ModelShader;
typedef enum ImageDataOffsets ImageDataOffsets;
typedef enum AssetType AssetType;
typedef enum ParamType ParamType;

// Struct forward declarations
typedef struct Viewmodel Viewmodel;
typedef struct GlobalState GlobalState;
typedef b2Vec2 Vector2;
typedef struct Camera Camera;
typedef struct Player Player;
typedef struct Wall Wall;
typedef struct Level Level;
typedef struct Actor Actor;
typedef struct Options Options;
typedef struct Asset Asset;
typedef struct Image Image;
typedef struct Font Font;
typedef struct SaveData SaveData;
typedef struct Color Color;
typedef struct ActorConnection ActorConnection;
typedef struct Param Param;
typedef struct ModelDefinition ModelDefinition;
typedef struct Material Material;
typedef struct ModelLod ModelLod;
typedef struct KvList KvList;

// Function signatures
typedef void (*FixedUpdateFunction)(GlobalState *state, double delta);

typedef void (*FrameUpdateFunction)(GlobalState *state);

typedef void (*FrameRenderFunction)(GlobalState *state);

typedef void (*ActorInitFunction)(Actor *this, b2WorldId worldId, const KvList *params);

typedef void (*ActorUpdateFunction)(Actor *this, double delta);

typedef void (*ActorIdleFunction)(Actor *this, double delta);

typedef void (*ActorTargetReachedFunction)(Actor *this, double delta);

typedef void (*ActorDestroyFunction)(Actor *this);

/**
 * Signal handler function signature for actor
 * @return True if the signal was handled, false if not
 */
typedef bool (*ActorSignalHandlerFunction)(Actor *this, const Actor *sender, byte signal, const Param *param);

#pragma endregion

#pragma region Utility defines

#define STR(x) #x
#define TO_STR(x) STR(x)
#define PHYSICS_TARGET_MS (1000 / PHYSICS_TARGET_TPS)
#define PHYSICS_TARGET_MS_D (1000.0 / PHYSICS_TARGET_TPS)
#define PHYSICS_TARGET_NS (1000000000 / PHYSICS_TARGET_TPS) // nanoseconds because precision
#define PHYSICS_TARGET_NS_D (1000000000.0 / PHYSICS_TARGET_TPS)

/// Use this for the "OK/Accept" button in place of hardcoding controller A or B buttons
#define CONTROLLER_OK (GetState()->options.controllerSwapOkCancel ? SDL_CONTROLLER_BUTTON_B : SDL_CONTROLLER_BUTTON_A)
/// Use this for the "Cancel" button in place of hardcoding controller A or B buttons
#define CONTROLLER_CANCEL \
	(GetState()->options.controllerSwapOkCancel ? SDL_CONTROLLER_BUTTON_A : SDL_CONTROLLER_BUTTON_B)

/**
 * Convert a hex color (0xAARRGGBB) to a Color struct (RGBA 0-1)
 * @param argb
 * @return The Color struct
 */
#define COLOR(argb) \
	(Color){((argb >> 16) & 0xFF) / 255.0f, \
			((argb >> 8) & 0xFF) / 255.0f, \
			((argb) & 0xFF) / 255.0f, \
			((argb >> 24) & 0xFF) / 255.0f}

/**
 * Convert a byte to a float (0-1)
 * Intended for converting color channel bytes to floats
 * @param byte The byte to convert
 * @return The float value
 */
#define BYTE_TO_FLOAT(byte) (byte / 255.0f)

/**
 * Convert a Color struct to an array of floats (cglm vec4)
 * @param color The color struct to convert
 * @return The color as an array of floats
 */
#define COLOR_TO_ARR(color) (float *)&(color)

#ifdef WIN32
/// Make this symbol exported (in the symbol table)
#define EXPORT_SYM __declspec(dllexport)
#else
/// Make this symbol exported (in the symbol table)
#define EXPORT_SYM __attribute__((visibility("default")))
#endif

#define PARAM_BYTE(x) ((Param){PARAM_TYPE_BYTE, .byteValue = x})
#define PARAM_INT(x) ((Param){PARAM_TYPE_INTEGER, .intValue = x})
#define PARAM_FLOAT(x) ((Param){PARAM_TYPE_FLOAT, .floatValue = x})
#define PARAM_BOOL(x) ((Param){PARAM_TYPE_BOOL, .boolValue = x})
#define PARAM_STRING(x) ((Param){PARAM_TYPE_STRING, .stringValue = x})
#define PARAM_COLOR(x) ((Param){PARAM_TYPE_COLOR, .colorValue = x})
#define PARAM_NONE ((Param){PARAM_TYPE_NONE})

#pragma endregion

#pragma region Standard Colors

#define COLOR_WHITE COLOR(0xFFFFFFFF)
#define COLOR_BLACK COLOR(0xFF000000)

#pragma endregion

#pragma region Enum definitions

enum AssetType
{
	ASSET_TYPE_TEXTURE = 0,
	ASSET_TYPE_WAV = 1,
	ASSET_TYPE_LEVEL = 2,
	ASSET_TYPE_SHADER = 3,
	ASSET_TYPE_MODEL = 4,
	ASSET_TYPE_FONT = 5
};

/**
 * Use to get data from a decompressed image asset using @c ReadUintA
 */
enum ImageDataOffsets
{
	IMAGE_SIZE_OFFSET = 0,
	IMAGE_WIDTH_OFFSET = 4,
	IMAGE_HEIGHT_OFFSET = 8,
	IMAGE_ID_OFFSET = 12
};

/**
 * Used to check which game state the game is in
 * Now, you *could* just set a complete mess of state functions and disregard this, but if you do that, I will find you.
 */
enum CurrentState
{
	LEVEL_SELECT_STATE,
	LOGO_SPLASH_STATE,
	MAIN_STATE,
	MENU_STATE,
	PAUSE_STATE,
	OPTIONS_STATE,
	VIDEO_OPTIONS_STATE,
	SOUND_OPTIONS_STATE,
	INPUT_OPTIONS_STATE,
	LOADING_STATE,
};

/**
 * Used to check which renderer the game is using
 */
enum Renderer
{
	RENDERER_OPENGL,
	RENDERER_VULKAN,
	RENDERER_MAX
};

/**
 * Used the check the MSAA level setting
 */
enum OptionsMsaa
{
	MSAA_NONE = 0,
	MSAA_2X = 1,
	MSAA_4X = 2,
	MSAA_8X = 3
};

/**
 * List of shaders a model can be rendered with
 */
enum ModelShader
{
	/// The sky shader. Do not use on in-level models.
	SHADER_SKY,
	/// A basic shader with no lighting
	SHADER_UNSHADED,
	/// A shader with basic lighting based on the vertex normals.
	SHADER_SHADED
};

enum CollisionGroups
{
	COLLISION_GROUP_DEFAULT = 1 << 0,
	COLLISION_GROUP_PLAYER = 1 << 1,
	COLLISION_GROUP_ACTOR = 1 << 2,
	COLLISION_GROUP_TRIGGER = 1 << 3,
	COLLISION_GROUP_ACTOR_ENEMY = 1 << 4,
	COLLISION_GROUP_HURTBOX = 1 << 5,
};

enum ParamType
{
	PARAM_TYPE_BYTE,
	PARAM_TYPE_INTEGER,
	PARAM_TYPE_FLOAT,
	PARAM_TYPE_BOOL,
	PARAM_TYPE_STRING,
	PARAM_TYPE_NONE,
	PARAM_TYPE_COLOR
};

#pragma endregion

#pragma region Struct definitions

struct Viewmodel
{
	bool enabled;
	ModelDefinition *model;
	uint modelSkin;
	vec3 translation;
	vec3 rotation;
};

struct KvList
{
	LockingList keys;
	LockingList values;
};

struct Color
{
	float r;
	float g;
	float b;
	float a;
};

struct Param
{
	ParamType type;
	union
	{
		byte byteValue;
		int intValue;
		float floatValue;
		bool boolValue;
		char stringValue[64];
		Color colorValue;
	};
};

struct Camera
{
	/// The X position of the camera
	float x;
	/// The Y position of the camera
	float y;
	/// The Z position of the camera
	float z;

	/// The pitch of the camera
	float pitch;
	/// The yaw of the camera
	float yaw;
	/// The roll of the camera
	float roll;

	/// The field of view of the camera
	float fov;
};

struct Player
{
	/// The player's position
	Vector2 pos;
	/// The player's rotation
	float angle;
	/// The player's Box2D body ID
	b2BodyId bodyId;
};

// Utility functions are in Structs/wall.h
struct Wall
{
	/// The first point of the wall
	Vector2 a;
	/// The second point of the wall
	Vector2 b;
	/// The fully qualified texture name (texture/level_uvtest.gtex instead of level_uvtest)
	const char tex[80];
	/// The length of the wall (Call @c WallBake to update)
	float length;
	/// The angle of the wall (Call @c WallBake to update)
	float angle;
	/// The change in x over the length of the wall, calculated with @code Wall.b.x - Wall.a.x@endcode
	float dx;
	/// The change in y over the length of the wall, calculated with @code Wall.b.y - Wall.a.y@endcode
	float dy;
	/// The X scale of the texture
	float uvScale;
	/// The X offset of the texture
	float uvOffset;
	/// height of the wall for rendering. Does not affect collision
	float height;
	/// The wall's Box2D body ID
	b2BodyId bodyId;
};

// Utility functions are in Structs/level.h
struct Level
{
	/// The list of actors in the level
	LockingList actors;
	/// The list of walls in the level
	List walls;

	/// Indicates if the level has a ceiling. If false, the level will use a sky instead
	bool hasCeiling;
	/// The fully qualified texture name (texture/level_uvtest.gtex instead of level_uvtest)
	char ceilOrSkyTex[80];
	/// The fully qualified texture name (texture/level_uvtest.gtex instead of level_uvtest)
	char floorTex[80];

	/// The music name, or "none" for no music
	char music[80];

	/// The color of the fog
	uint fogColor;
	/// The distance from the player at which the fog begins to fade in
	float fogStart;
	/// The distance from the player at which the fog is fully opaque
	float fogEnd;

	/// The ID of the Box2D world
	b2WorldId worldId;

	/// The player object
	Player player;

	/// The map of named actors in the level (key portion)
	LockingList namedActorNames;
	/// The map of named actors in the level (value portion)
	List namedActorPointers;

	/// A pointer to the I/O proxy actor, if it exists
	Actor *ioProxy;
};

struct Options
{
	/// Checksum of the options struct (helps prevent corruption)
	ushort checksum;

	/* Controls */

	/// Whether the game is in controller mode
	bool controllerMode;
	/// The look speed (it affects controller speed too)
	double mouseSpeed;
	/// The strength of the rumble
	float rumbleStrength;
	/// Whether to invert the camera X axis (controller only)
	bool cameraInvertX;
	/// Whether to swap the controller A and B buttons
	bool controllerSwapOkCancel;

	/* Video */

	/// The renderer to use
	Renderer renderer;
	/// Whether the game is fullscreen
	bool fullscreen;
	/// Whether vsync is enabled
	bool vsync;
	/// The MSAA level
	OptionsMsaa msaa;
	/// Whether to use mipmaps
	bool mipmaps;
	/// Whether to prefer Wayland over X11
	bool preferWayland;
	/// Whether to drop to 30 fps when the window is not focused
	bool limitFpsWhenUnfocused;
	/// The LOD distance multiplier
	float lodMultiplier;

	/* Audio */

	/// The volume of the music
	double musicVolume;
	/// The volume of the sound effects
	double sfxVolume;
	/// The master volume
	double masterVolume;
} __attribute__((packed)); // This is packed because it is saved to disk

// Global state of the game
struct GlobalState
{
	/// Current level
	Level *level;

	/// State update function
	FrameUpdateFunction UpdateGame;
	/// State render function
	FrameRenderFunction RenderGame;
	/// The current state of the game
	CurrentState currentState;
	/// The number of physics frames that have passed since the last game state change
	ulong physicsFrame;

	/// The save data (persists between levels)
	SaveData *saveData;

	/// The camera
	Camera *cam;
	/// The Y position of the camera
	double cameraY;
	/// The scale of the UI.
	double uiScale;

	Viewmodel viewmodel;

	/// Game options
	Options options;
	/// Whether the audio system has been started successfully
	bool isAudioStarted;
	/// background music
	Mix_Music *music;
	/// sound effects
	Mix_Chunk *channels[SFX_CHANNEL_COUNT];

	/// The path to the executable
	char executablePath[261];
	/// The path to the executable folder
	char executableFolder[261];
	/// Whether to freeze the event loop. This should only be used for debugging.
	bool freezeEvents;
	/// Request to exit the game
	bool requestExit;
};

struct SaveData
{
	/// Player health
	int hp;
	/// The number of coins the player has
	int coins;
	/// The number of blue coins the player has
	int blueCoins;
};

// Actor (interactable/moving wall) struct
struct Actor
{
	/// The center position of the actor
	/// @warning This is the visual position only, and synchronization between the visual position and the physics
	///	 position must be explicitly done in the update function
	Vector2 position;
	/// The rotation of the actor
	/// @warning This is the visual rotation only, and synchronization between the visual rotation and the physics
	///	 rotation must be explicitly done in the update function
	float rotation;
	/// Y position for rendering
	/// @note Because this game uses a 2d physics engine, this value is not considered in any physics calculations.
	///  As such, an actor can be rendered floating, but will always collide as though it is on the same plane as
	///  everything else.
	float yPosition;
	/// Optional model for the actor, if not NULL, will be rendered instead of the wall
	ModelDefinition *actorModel;
	/// The index of the active skin for the actor's model
	uint currentSkinIndex;
	/// The current LOD level of the actor's model, re-calculated each physics tick
	uint currentLod;

	/// The actor's wall, in global space
	Wall *actorWall;

	/// The actor type index
	/// @warning Do not change this after creation
	uint actorType;
	/// The function to call when the actor is initialized
	/// @note This should only be called once, when the actor is created
	ActorInitFunction Init;
	/// The function to call when the actor is updated
	/// @note This should be called every tick
	ActorUpdateFunction Update;
	/// The function to call when the actor is destroyed
	/// @note This should only be called once, when the actor is destroyed
	ActorDestroyFunction Destroy;
	/// The function to call when the actor receives a signal.
	ActorSignalHandlerFunction SignalHandler;
	/// List of I/O connections
	LockingList ioConnections;

	/// The actor's health
	/// @note May be unused for some actors
	int health;
	/// Extra data for the actor
	void *extraData;

	/// The actor's Box2D body ID
	b2BodyId bodyId;
};

struct Asset
{
	/// The compressed size of the asset, excluding the header
	size_t compressedSize;
	/// The decompressed size of the asset
	size_t size;
	/// The type of the asset
	AssetType type;
	/// The version of the type
	uint8_t typeVersion;
	/// The data of the asset
	byte *data;
};

struct Image
{
	/// The width of the image
	size_t width;
	/// The height of the image
	size_t height;
	/// The ID of the image. This is generated at runtime and not consistent between runs.
	uint id;

	bool filter;
	bool repeat;
	bool mipmaps;

	/// The name of the image
	char *name;
	/// The pixel data of the image
	byte *pixelData;
};

struct Font
{
	/// The texture width of one character
	uint8_t width;
	/// The texture height (including below baseline)
	uint8_t textureHeight;
	/// The pixel coordinate of the baseline
	uint8_t baseline;
	/// The pixels between characters
	uint8_t charSpacing;
	/// The pixels between lines
	uint8_t lineSpacing;
	/// The width of a space character
	uint8_t spaceWidth;
	/// The default size of the font, used for calculating scale
	uint8_t defaultSize;
	/// The number of characters in the font
	uint8_t charCount;
	/// Whether this font only contains uppercase characters
	bool uppercaseOnly;

	/// The texture this font uses (fully qualified)
	char* texture;
	/// The index of the character in the texture
	uint8_t indices[255];
	/// The width of each character, index directly by the character
	uint8_t charWidths[255];

	/// The image loaded from the texture
	Image *image;
} __attribute__((packed));

struct ActorConnection
{
	byte targetInput;
	byte myOutput;
	char outActorName[64];
	Param outParamOverride;
};

struct Material
{
	/// The runtime-generated ID of this model
	size_t id;

	/// The texture name of the material
	char *texture;
	/// The tint color of the material
	Color color;
	/// The shader to use for this material
	ModelShader shader;
};

struct ModelLod
{
	/// The runtime-generated ID of this model
	size_t id;

	/// How far away the camera must be before this LOD is used
	float distance;

	/// The number of vertices in the model
	size_t vertexCount;
	/// Packed vertex data, (X Y Z) (U V) (R G B A) (NX NY NZ)
	float *vertexData;

	/// The total number of indices across all materials
	uint totalIndexCount;
	/// The number of indices in each material
	uint *indexCount;
	/// Index data for each material
	uint **indexData;
};

struct ModelDefinition
{
	/// The runtime-generated ID of this model
	size_t id;
	/// The asset name of this model
	char *name;

	/// The number of materials in the model
	size_t materialCount;

	size_t materialsPerSkin;

	/// The number of skins in the model
	size_t skinCount;
	/// The number of LODs in the model
	size_t lodCount;

	Material *materials;
	/// The skins for this model, each an array of materialsPerSkin indices into the materials array
	size_t **skins;
	/// The LODs for this model
	ModelLod **lods;
};

#pragma endregion

#endif //GAME_DEFINES_H
