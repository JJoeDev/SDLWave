#pragma once
#include <cmath>

#ifndef PI
#define PI 3.14159265358979323846f
#endif

namespace sinFuncs {
	double getY(int time, int x, int width, int height, float frequency) {
		return (std::sin((static_cast<double>(x) + time) / width * frequency * PI) + 1) / 2 * height + 10;
	}
}