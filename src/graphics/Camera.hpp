#ifndef WINDOW_CAMERA_HPP
#define WINDOW_CAMERA_HPP

#include "glm/ext/vector_double3.hpp"
#include <glm/glm.hpp>

using namespace glm;

class Camera {
	const double REBASE_GRANULARITY = 1000.0;

	void updateVectors();

	dvec3 x_dir;
	dvec3 y_dir;
	dvec3 z_dir;
	
	dvec3 originPosition;
	dvec3 offsetPosition;
	dvec3 accumulatedShift = dvec3(0.0);

	bool rebased_dirty = true;

	float fov;
	mat4 rotation;
	
	dvec3 getRebaseShift();
public:
	Camera(dvec3 position, float fov);

	inline void setxdir(const glm::dvec3 &dir) {x_dir = dir;}
	inline void setydir(const glm::dvec3 &dir) {y_dir = dir;}
	inline void setzdir(const glm::dvec3 &dir) {z_dir = dir;}

	inline const dvec3& xdir() {return x_dir;}
	inline const dvec3& ydir() {return y_dir;}
	inline const dvec3& zdir() {return z_dir;}

	inline void setRotation(const glm::mat4 &rot) {rotation = rot;}

	inline dvec3& getPosition() {return originPosition;}

	void translate(const glm::dvec3 &dp);
	void set(double x, double y, double z);
	void rotate(float x, float y, float z);

	mat4 getProjection();
	mat4 getView();
	
	inline const dvec3& getWorldShift() const {return accumulatedShift;};
	void originRebase();

	inline bool isRebased() {return rebased_dirty;}
};

#endif /* WINDOW_CAMERA_H_ */