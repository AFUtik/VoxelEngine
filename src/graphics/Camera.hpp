#ifndef WINDOW_CAMERA_HPP
#define WINDOW_CAMERA_HPP

#include "glm/ext/vector_double3.hpp"
#include <glm/glm.hpp>

using namespace glm;

class Camera {
	void updateVectors();
public:
	dvec3 x_dir;
	dvec3 y_dir;
	dvec3 z_dir;

	dvec3 originPosition;
	float fov;
	mat4 rotation;
	Camera(dvec3 position, float fov);

	inline dvec3& getPosition() {return originPosition;}

	void set_xyz(double x, double y, double z);
	void rotate(float x, float y, float z);

	mat4 getProjection();
	mat4 getView();
	
	dvec3 getRebaseShift();
	void originRebase();
};

#endif /* WINDOW_CAMERA_H_ */