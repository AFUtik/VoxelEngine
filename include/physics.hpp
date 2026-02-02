#pragma once

#include "ptypes.hpp"

//struct Collider {
//
//};

struct AABB {
	Vector3 min = Vector3(0), max = Vector3(0);

	AABB() {}
	AABB(Vector3 min, Vector3 max) : min(min), max(max) {}

	inline void translate(const Vector3& vector) {
		min += vector; max += vector;
	}
};

struct OBB {
	Vector3 center;
	Vector3 halfSize;
	mat3 rotation = mat3(1.0f);

	OBB(Vector3 center, Vector3 half) : center(center), halfSize(half) {}
};