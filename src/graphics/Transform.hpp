#ifndef TRANSFORM_HPP
#define TRANSFORM_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Camera.hpp"

class Transform {
    glm::dvec3 originPosition;

    glm::quat rotation;
    glm::vec3 scale;

    glm::mat4 cached_model;
    bool dirty = true;
public:
    Transform() : scale(1.0f), rotation(1,0,0,0), originPosition(0) {}

    inline const glm::dvec3& getPosition() const {return originPosition;}
    inline const glm::quat&  getRotation() const {return rotation;}
    inline const glm::vec3&  getScale()    const {return scale;}

    inline void setRotation(const glm::quat &q) { rotation = q; dirty = true; }
    inline void setScale(const glm::vec3 &s)    { scale = s;   dirty = true; }
    inline void setPosition(const glm::dvec3 &p){ originPosition = p;dirty = true; }

    inline void translate(const glm::dvec3 &dp) { originPosition += dp; dirty = true; }

    inline const glm::mat4 model(Camera* camera) {
        if (camera->isRebased() || dirty) {
            glm::mat4 m(1.0f);
            glm::dvec3 OffsetPosition = originPosition - camera->getWorldShift();

            // Translate
            m[3][0] = (float)(OffsetPosition.x);
            m[3][1] = (float)(OffsetPosition.y);
            m[3][2] = (float)(OffsetPosition.z);

            // Rotate
            m = m * glm::mat4_cast(rotation);

            // Scale
            m[0] *= scale.x;
            m[1] *= scale.y;
            m[2] *= scale.z;
            
            cached_model = m;
            dirty = false;
            return m;
        }
        else return cached_model;
    }
};

#endif