//
// Created by droc101 on 4/21/2024.
//

#include "MathEx.h"
#include <math.h>
#include <stdint.h>

int wrapi(const int x, const int min, const int max)
{
	if (min > max)
	{
		return wrapi(x, max, min);
	}
	return (x >= 0 ? min : max) + x % max - min;
}

uint32_t wrapu(const uint32_t x, const uint32_t min, const uint32_t max)
{
	return min > max ? x % min : x % max;
}

float wrapf(const float x, const float min, const float max)
{
	if (min > max)
	{
		return wrapf(x, max, min);
	}
	return (x >= 0 ? min : max) + fmodf(x, max - min);
}

double wrapd(const double x, const double min, const double max)
{
	if (min > max)
	{
		return wrapd(x, max, min);
	}
	return (x >= 0 ? min : max) + fmod(x, max - min);
}
