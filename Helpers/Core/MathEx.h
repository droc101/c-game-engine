//
// Created by droc101 on 4/21/2024.
//

#ifndef GAME_MATHEX_H
#define GAME_MATHEX_H

#include <stdint.h>

#define PI 3.14159265358979323846
#define PIf 3.14159265358979323846f

/**
 * Returns the minimum of two numbers
 * @param a Number 1
 * @param b Number 2
 * @return The smaller of the two numbers
 */
#define min(a, b) ((a) < (b) ? (a) : (b))

/**
 * Returns the maximum of two numbers
 * @param a Number 1
 * @param b Number 2
 * @return The larger of the two numbers
 */
#define max(a, b) ((a) < (b) ? (b) : (a))

int wrapi(int x, int min, int max);

uint32_t wrapu(uint32_t x, uint32_t min, uint32_t max);

float wrapf(float x, float min, float max);

double wrapd(double x, double min, double max);

/**
 * Wraps a number between a minimum and maximum value
 * @param x Number to wrap
 * @param min Minimum value
 * @param max Maximum value
 * @return The wrapped number
 */
#define wrap(x, min, max) _Generic((x), float: wrapf, double: wrapd, uint32_t: wrapu, default: wrapi)(x, min, max)

/**
 * Remap a number from one range to another
 * @param in Input number
 * @param inMin Input minimum
 * @param inMax Input maximum
 * @param outMin Output minimum
 * @param outMax Output maximum
 * @return Input value remapped to the output range
 */
#define remap(in, inMin, inMax, outMin, outMax) \
	(((in) - (inMin)) * ((outMax) - (outMin)) / ((inMax) - (inMin)) + (outMin))

/**
 * Linear interpolation between two numbers
 * @param a Number 1
 * @param b Number 2
 * @param factor Interpolation factor
 * @return Interpolated value
 */
#define lerp(a, b, factor) ((a) * (_Generic((factor), default: 1.0, float: 1.0f) - (factor)) + (b) * (factor))

/**
 * Clamp a number between a minimum and maximum value
 * @param val Number to clamp
 * @param min Minimum value
 * @param max Maximum value
 * @return Clamped number
 */
#define clamp(val, min, max) ((val) < (min) ? (min) : (val) > (max) ? (max) : (val))

/**
 * Convert degrees to radians
 * @param d Degrees
 */
#define degToRad(d) ((d) * PI / 180)

/**
 * Convert radians to degrees
 * @param r Radians
 */
#define radToDeg(r) ((r) * 180 / PI)

#endif //GAME_MATHEX_H
