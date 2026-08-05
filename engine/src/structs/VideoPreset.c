//
// Created by droc101 on 8/5/26.
//

#include <engine/graphics/RenderingHelpers.h>
#include <engine/structs/Options.h>
#include <engine/structs/VideoPreset.h>
#include <stdbool.h>

typedef struct VideoPresetData
{
	OptionsMsaa msaa;
	bool mipmaps;
	float lodMultiplier;
	OptionsAnisotropy anisotropy;
	OptionsShadowMapResolution shadowMapQuality;
} VideoPresetData;

static const VideoPresetData VERY_LOW_PRESET = {
	.msaa = MSAA_NONE,
	.mipmaps = false,
	.lodMultiplier = 0.5f,
	.anisotropy = ANISOTROPY_NONE,
	.shadowMapQuality = SHADOW_MAP_RESOLUTION_DISABLED,
};

static const VideoPresetData LOW_PRESET = {
	.msaa = MSAA_2X,
	.mipmaps = true,
	.lodMultiplier = 1.0f,
	.anisotropy = ANISOTROPY_4X,
	.shadowMapQuality = SHADOW_MAP_RESOLUTION_512,
};

static const VideoPresetData MEDIUM_PRESET = {
	.msaa = MSAA_4X,
	.mipmaps = true,
	.lodMultiplier = 1.0f,
	.anisotropy = ANISOTROPY_8X,
	.shadowMapQuality = SHADOW_MAP_RESOLUTION_2048,
};

static const VideoPresetData HIGH_PRESET = {
	.msaa = MSAA_8X,
	.mipmaps = true,
	.lodMultiplier = 1.5f,
	.anisotropy = ANISOTROPY_16X,
	.shadowMapQuality = SHADOW_MAP_RESOLUTION_4096,
};

static const VideoPresetData ULTRA_PRESET = {
	.msaa = MSAA_8X,
	.mipmaps = true,
	.lodMultiplier = 2.0f,
	.anisotropy = ANISOTROPY_16X,
	.shadowMapQuality = SHADOW_MAP_RESOLUTION_8192,
};

static bool IsPresetDataActive(const VideoPresetData *preset, const Options *options)
{
	return options->msaa == preset->msaa &&
		   options->mipmaps == preset->mipmaps &&
		   options->lodMultiplier == preset->lodMultiplier &&
		   options->anisotropy == preset->anisotropy &&
		   options->shadowMapQuality == preset->shadowMapQuality;
}

static void ApplyPresetData(const VideoPresetData *preset, Options *options)
{
	options->msaa = preset->msaa;
	options->mipmaps = preset->mipmaps;
	options->lodMultiplier = preset->lodMultiplier;
	options->anisotropy = preset->anisotropy;
	options->shadowMapQuality = preset->shadowMapQuality;
}

VideoPreset GetCurrentVideoPreset(const Options *options)
{
	if (IsPresetDataActive(&VERY_LOW_PRESET, options))
	{
		return VIDEO_PRESET_VERY_LOW;
	}
	if (IsPresetDataActive(&LOW_PRESET, options))
	{
		return VIDEO_PRESET_LOW;
	}
	if (IsPresetDataActive(&MEDIUM_PRESET, options))
	{
		return VIDEO_PRESET_MEDIUM;
	}
	if (IsPresetDataActive(&HIGH_PRESET, options))
	{
		return VIDEO_PRESET_HIGH;
	}
	if (IsPresetDataActive(&ULTRA_PRESET, options))
	{
		return VIDEO_PRESET_ULTRA;
	}
	return VIDEO_PRESET_CUSTOM;
}

void ApplyVideoPreset(Options *options, const VideoPreset preset)
{
	switch (preset)
	{
		case VIDEO_PRESET_VERY_LOW:
			ApplyPresetData(&VERY_LOW_PRESET, options);
			break;
		case VIDEO_PRESET_LOW:
			ApplyPresetData(&LOW_PRESET, options);
			break;
		case VIDEO_PRESET_MEDIUM:
			ApplyPresetData(&MEDIUM_PRESET, options);
			break;
		case VIDEO_PRESET_HIGH:
			ApplyPresetData(&HIGH_PRESET, options);
			break;
		case VIDEO_PRESET_ULTRA:
			ApplyPresetData(&ULTRA_PRESET, options);
			break;
		case VIDEO_PRESET_CUSTOM:
			// nop
			break;
	}
}
