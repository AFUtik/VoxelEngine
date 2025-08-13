#ifndef TRANSFORM_HPP
#define TRANSFORM_HPP

#include "glm/ext/quaternion_transform.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

class Transform {
    glm::dvec3 OriginPosition;
    glm::vec3  OffsetPosition;

    glm::quat rotation;
    glm::vec3 scale;

    glm::mat4 cached_model;
    bool dirty = true;
public:
    Transform() : scale(1.0f), rotation(1,0,0,0), OriginPosition(0), OffsetPosition(0) {}

    inline void setRotation(const glm::quat &q) { rotation = q; dirty = true; }
    inline void setScale(const glm::vec3 &s)    { scale = s;   dirty = true; }
    inline void setPosition(const glm::dvec3 &p){ OriginPosition = p;dirty = true; }

    inline void translate(const glm::dvec3 &dp) { OriginPosition += dp; dirty = true; }

    inline const glm::mat4 model(const glm::dvec3 &camera_pos) {
        glm::mat4 m(1.0f);

        // Translate
        m[3][0] = (float)(OffsetPosition.x - camera_pos.x);
        m[3][1] = (float)(OffsetPosition.y - camera_pos.y);
        m[3][2] = (float)(OffsetPosition.z - camera_pos.z);

        // Rotate
        m = m * glm::mat4_cast(rotation);

        // Scale
        m[0] *= scale.x;
        m[1] *= scale.y;
        m[2] *= scale.z;

        return m;
    }
};

#endif