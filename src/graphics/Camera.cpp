#include "Camera.hpp"
#include "../window/Window.hpp"

#include <glm/ext.hpp>
#include <iostream>

Camera::Camera(dvec3 position, float fov) : originPosition(position), fov(fov), rotation(1.0f) {
	updateVectors();
}

void Camera::set(double x, double y, double z) {
	originPosition = glm::vec3(x, y, z);
}

void Camera::updateVectors() {
	x_dir = dvec3(rotation * vec4(1, 0, 0, 1));
	y_dir = dvec3(rotation * vec4(0, 1, 0, 1));
	z_dir = dvec3(rotation * vec4(0, 0, -1, 1));
}

void Camera::rotate(float x, float y, float z) {
	rotation = glm::rotate(rotation, z, vec3(0, 0, 1));
	rotation = glm::rotate(rotation, y, vec3(0, 1, 0));
	rotation = glm::rotate(rotation, x, vec3(1, 0, 0));

	updateVectors();
}

void Camera::translate(const glm::dvec3 &dp) {
	originPosition += dp;
}

mat4 Camera::getProjection() {
	float aspect = (float)Window::width / (float)Window::height;
	return glm::perspective(fov, aspect, 0.1f, 1000.0f);
}

const mat4& Camera::updateView() {
	view = glm::lookAt(offsetPosition, offsetPosition + z_dir, y_dir);
	return view;
}

vec3 Camera::getViewDir() {
	vec3 forward = -vec3(view[0][2], view[1][2], view[2][2]);
    return glm::normalize(forward);
}

dvec3 Camera::getRebaseShift() {
	return glm::trunc((originPosition-accumulatedShift) / REBASE_GRANULARITY) * REBASE_GRANULARITY;
}

void Camera::originRebase() {
    dvec3 shift = getRebaseShift();
    if (shift == dvec3(0.0)) {
		offsetPosition = originPosition - accumulatedShift;
		rebased_dirty = false;
		return;
	}
	accumulatedShift += shift;
	rebased_dirty = true;
}