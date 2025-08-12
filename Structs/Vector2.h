//
// Created by droc101 on 4/21/2024.
//

// ReSharper disable CppRedundantInlineSpecifier
#ifndef GAME_VECTOR2_H
#define GAME_VECTOR2_H

#include "../defines.h"

/**
 * Create a 2D vector with the given x and y values
 * @param x The x value
 * @param y The y value
 * @return The new vector
 */
#define v2(x, y) ((Vector2){(x), (y)})

/**
 * Create a 2D vector with the same x and y values
 * @param xy The x and y value
 * @return The new vector
 */
#define v2s(xy) v2((xy), (xy))

/**
 * Create a unit vector from an angle
 * @param angle The angle (in radians)
 * @return The unit vector
 */
#define Vector2FromAngle(angle) v2(-sinf(angle), -cosf(angle))

/**
 * Get the distance between two vectors
 * @param a The first vector
 * @param b The second vector
 * @return The distance between the two vectors
 */
#define Vector2Distance(a, b) \
	({ \
		static_assert(sizeof(a) == sizeof(Vector2) && sizeof(b) == sizeof(Vector2)); \
		const float dx = (b).x - (a).x; \
		const float dy = (b).y - (a).y; \
		sqrtf(dx * dx + dy * dy); \
	})

/**
 * Get the length of a vector
 * @param vector The vector
 * @return The length of the vector
 */
#define Vector2Length(vector) \
	({ \
		static_assert(sizeof(vector) == sizeof(Vector2)); \
		sqrtf(((vector).x * (vector).x) + ((vector).y * (vector).y)); \
	})

/**
 * Convert a vector into a unit vector if possible
 * @param vector The vector
 * @return The unit vector, or the zero vector if @c vector cannot be turned into a unit vector.
 */
#define Vector2Normalize(vector) \
	({ \
		static_assert(sizeof(vector) == sizeof(Vector2)); \
		const float length = Vector2Length(vector); \
		v2((vector).x / length, (vector).y / length); \
	})

/**
 * Add two vectors
 * @param a The first vector
 * @param b The second vector
 * @return The sum of the two vectors
 */
#define Vector2Add(a, b) \
	({ \
		static_assert(sizeof(a) == sizeof(Vector2) && sizeof(b) == sizeof(Vector2)); \
		v2((a).x + (b).x, (a).y + (b).y); \
	})

/**
 * Subtract two vectors
 * @param a The vector
 * @param b The offset
 * @return The difference of the two vectors
 */
#define Vector2Sub(a, b) \
	({ \
		static_assert(sizeof(a) == sizeof(Vector2) && sizeof(b) == sizeof(Vector2)); \
		v2((a).x - (b).x, (a).y - (b).y); \
	})

/**
 * Rotate a vector by an angle
 * @param vector The vector
 * @param angle The angle (in radians)
 * @return The rotated vector
 */
#define Vector2Rotate(vector, angle) \
	({ \
		static_assert(sizeof(vector) == sizeof(Vector2)); \
		const float x = cosf(angle); \
		const float y = sinf(angle); \
		v2(x * (vector).x - y * (vector).y, y * (vector).x + x * (vector).y); \
	})

/**
 * Get the dot product of two vectors
 * @param a The first vector
 * @param b The second vector
 * @return The dot product of the two vectors
 */
#define Vector2Dot(a, b) \
	({ \
		static_assert(sizeof(a) == sizeof(Vector2) && sizeof(b) == sizeof(Vector2)); \
		((a).x * (b).x + (a).y * (b).y); \
	})

/**
 * Get the angle between two vectors
 * @param a The first vector
 * @param b The second vector
 * @return The angle between the two vectors (in radians)
 */
#define Vector2Angle(a, b) \
	({ \
		static_assert(sizeof(a) == sizeof(Vector2) && sizeof(b) == sizeof(Vector2)); \
		acosf(Vector2Dot(a, b) / (Vector2Length(a) * Vector2Length(b))); \
	})

/**
 * Scale a vector (multiply by a scalar)
 * @param vector The vector
 * @param scale The scalar
 * @return The scaled vector
 */
#define Vector2Scale(vector, scale) \
	({ \
		static_assert(sizeof(vector) == sizeof(Vector2)); \
		v2((scale) * (vector).x, (scale) * (vector).y); \
	})

/**
 * Divide a vector by a scalar divisor
 * @param vector The vector
 * @param divisor The divisor
 * @return The divided vector
 * @note Prefer to scale by @code 1 / divisor@endcode instead of calling this function
 */
#define Vector2Div(vector, divisor) \
	({ \
		static_assert(sizeof(vector) == sizeof(Vector2)); \
		Vector2Scale(vector, (1 / (divisor))); \
	})

#define Vector2RelativeAngle(a, b) \
	({ \
		static_assert(sizeof(a) == sizeof(Vector2) && sizeof(b) == sizeof(Vector2)); \
		atan2f((b).x * (a).x - (b).x * (a).y, (b).y * (a).x + (b).y * (a).y); \
	})

#endif //GAME_VECTOR2_H
