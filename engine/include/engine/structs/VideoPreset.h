//
// Created by droc101 on 8/5/26.
//

#ifndef GAME_VIDEOPRESET_H
#define GAME_VIDEOPRESET_H

// forward decl.
typedef struct Options Options;

typedef enum VideoPreset
{
	VIDEO_PRESET_VERY_LOW,
	VIDEO_PRESET_LOW,
	VIDEO_PRESET_MEDIUM,
	VIDEO_PRESET_HIGH,
	VIDEO_PRESET_ULTRA,
	VIDEO_PRESET_CUSTOM,
} VideoPreset;

/**
 * Get the current video preset
 */
VideoPreset GetCurrentVideoPreset(const Options *options);

/**
 * Apply a video preset
 */
void ApplyVideoPreset(Options *options, VideoPreset preset);

#endif //GAME_VIDEOPRESET_H
