#include "Camera.hpp"
#include "../window/Window.hpp"

#include <glm/ext.hpp>

Camera::Camera(dvec3 position, float fov) : originPosition(position), fov(fov), rotation(1.0f) {
	updateVectors();
}

void Camera::set_xyz(double x, double y, double z) {
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

mat4 Camera::getProjection() {
	float aspect = (float)Window::width / (float)Window::height;
	return glm::perspective(fov, aspect, 0.1f, 500.0f);
}

mat4 Camera::getView() {
	return glm::lookAt(originPosition, originPosition + z_dir, y_dir);
}

dvec3 Camera::getRebaseShift() {
	const double REBASE_GRANULARITY = 1000.0;

	return glm::floor(originPosition / REBASE_GRANULARITY) * REBASE_GRANULARITY;
}

void Camera::originRebase() {
	dvec3 shift = getRebaseShift();
	if (shift == dvec3(0.0)) return;
	originPosition -= shift;
}