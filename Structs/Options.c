//
// Created by droc101 on 10/27/24.
//

#include "Options.h"
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../Helpers/Core/Error.h"
#include "../Helpers/Core/Logging.h"
#include "../Helpers/Graphics/RenderingHelpers.h"
#include "GlobalState.h"

void DefaultOptions(Options *options)
{
	options->renderer = RENDERER_VULKAN;
	options->musicVolume = 1.0f;
	options->sfxVolume = 1.0f;
	options->masterVolume = 1.0f;
	options->fullscreen = false;
	options->vsync = false;
	options->lodMultiplier = 1.0f;
	options->cameraSpeed = 1;
	options->controllerMode = false;
	options->msaa = MSAA_4X;
	options->mipmaps = true;
	options->rumbleStrength = 1.0f;
	options->invertHorizontalCamera = false;
	options->invertVerticalCamera = false;
	options->controllerSwapOkCancel = false;
	options->preferWayland = false;
	options->limitFpsWhenUnfocused = true;
}

bool ValidateOptions(const Options *options)
{
	if (options->renderer >= RENDERER_MAX)
	{
		return false;
	}
	if (options->musicVolume < 0 || options->musicVolume > 1)
	{
		return false;
	}
	if (options->sfxVolume < 0 || options->sfxVolume > 1)
	{
		return false;
	}
	if (options->masterVolume < 0 || options->masterVolume > 1)
	{
		return false;
	}
	if (options->cameraSpeed < 0.01 || options->cameraSpeed > 2.00)
	{
		return false;
	}
	return true;
}

uint16_t GetOptionsChecksum(Options *options)
{
	const uint8_t *data = (uint8_t *)options;
	uint16_t checksum = 0;
	for (size_t i = sizeof(uint16_t); i < sizeof(Options) - sizeof(uint16_t); i++)
	{
		checksum += data[i];
	}
	return checksum;
}

char *GetOptionsPath()
{
	const char *folderPath = GetState()->executableFolder;
	const char *fileName = "/options.bin";
	char *filePath = malloc(strlen(folderPath) + strlen(fileName) + 1);
	CheckAlloc(filePath);
	strcpy(filePath, folderPath);
	strcat(filePath, fileName);
	return filePath;
}

void LoadOptions(Options *options)
{
	char *filePath = GetOptionsPath();

	FILE *file = fopen(filePath, "rb");
	if (file == NULL)
	{
		LogWarning("Options file not found, using default options\n");
		DefaultOptions(options);
	} else
	{
		fseek(file, 0, SEEK_END);
		const size_t fileLen = ftell(file);

		// if the file is the wrong size, just use the default options
		if (fileLen != sizeof(Options))
		{
			LogWarning("Options file is invalid, using defaults\n");
			DefaultOptions(options);
			fclose(file);
			free(filePath);
			return;
		}

		LogInfo("Valid options file found, loading options\n");

		fseek(file, 0, SEEK_SET);
		const size_t bytesRead = fread(options, 1, sizeof(Options), file);
		if (bytesRead != sizeof(Options))
		{
			LogWarning("Failed to read options file, using defaults (got %d bytes, expected %d)\n",
					   bytesRead,
					   sizeof(Options));
			DefaultOptions(options);
		} else if (options->checksum !=
				   GetOptionsChecksum(options)) // This is an else because defaultOptions does not set the checksum
		{
			LogWarning("Options file checksum invalid, using defaults\n");
			DefaultOptions(options);
		}

		if (!ValidateOptions(options))
		{
			LogWarning("Options file is invalid, using defaults\n");
			DefaultOptions(options);
		}

		fclose(file);
	}

	free(filePath);
}

void SaveOptions(Options *options)
{
	options->checksum = GetOptionsChecksum(options);
	char *filePath = GetOptionsPath();

	FILE *file = fopen(filePath, "wb");
	if (file == NULL)
	{
		LogError("File opening failed: %s\n", strerror(errno));
		free(filePath);
		return;
	}
	fwrite(options, sizeof(Options), 1, file);
	fclose(file);

	free(filePath);
}
