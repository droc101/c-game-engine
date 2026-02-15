//
// Created by droc101 on 8/13/25.
//

#ifndef GAME_COLOR_H
#define GAME_COLOR_H

/**
 * Convert a hex color (0xAARRGGBB) to a Color struct (RGBA 0-1)
 * @param argb
 * @return The Color struct
 */
#define COLOR(argb) \
	(Color){(((argb) >> 16) & 0xFF) / 255.0f, \
			(((argb) >> 8) & 0xFF) / 255.0f, \
			((argb) & 0xFF) / 255.0f, \
			(((argb) >> 24) & 0xFF) / 255.0f}

/**
 * Convert a byte to a float (0-1)
 * Intended for converting color channel bytes to floats
 * @param byte The byte to convert
 * @return The float value
 */
#define BYTE_TO_FLOAT(byte) ((byte) / 255.0f)

/**
 * Convert a Color struct to an array of floats (cglm vec4)
 * @param color The color struct to convert
 * @return The color as an array of floats
 */
#define COLOR_TO_ARR(color) ((float *)&(color))

#define COLOR_WHITE COLOR(0xFFFFFFFF)
#define COLOR_BLACK COLOR(0xFF000000)

typedef struct Color Color;

struct Color
{
	/// The red component
	float r;
	/// The green component
	float g;
	/// The blue component
	float b;
	/// The alpha component
	float a;
};

#endif //GAME_COLOR_H
