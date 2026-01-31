#pragma once

#include "ptypes.hpp"

struct AABB {
	Vector3 min, max;
	AABB(Vector3 min, Vector3 max) : min(min), max(max) {}
};
