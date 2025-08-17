#ifndef FRUSTUM_HPP
#define FRUSTUM_HPP

#include <glm/glm.hpp>

struct Plane {
    glm::vec3 normal;
    float d;

    // проверка, точка слева или справа от плоскости
    float distance(const glm::vec3& point) const {
        return glm::dot(normal, point) + d;
    }
};

class Frustum {
public:
    Plane planes[6]; // left, right, top, bottom, near, far

    void update(const glm::mat4& m) {
        glm::vec4 row0 = glm::vec4(m[0][0], m[1][0], m[2][0], m[3][0]);
        glm::vec4 row1 = glm::vec4(m[0][1], m[1][1], m[2][1], m[3][1]);
        glm::vec4 row2 = glm::vec4(m[0][2], m[1][2], m[2][2], m[3][2]);
        glm::vec4 row3 = glm::vec4(m[0][3], m[1][3], m[2][3], m[3][3]);

        glm::vec4 left   = row3 + row0;
        glm::vec4 right  = row3 - row0;
        glm::vec4 bottom = row3 + row1;
        glm::vec4 top    = row3 - row1;
        glm::vec4 near   = row3 + row2;
        glm::vec4 far    = row3 - row2;

        auto setPlane = [&](int i, const glm::vec4& p) {
            planes[i].normal = glm::vec3(p.x, p.y, p.z);
            planes[i].d = p.w;
            float len = glm::length(planes[i].normal);
            planes[i].normal /= len;
            planes[i].d /= len;
        };

        setPlane(0, left);
        setPlane(1, right);
        setPlane(2, bottom);
        setPlane(3, top);
        setPlane(4, near);
        setPlane(5, far);
    }

    // Проверка точки
    bool pointInFrustum(const glm::vec3& point) const {
        for (int i = 0; i < 6; i++) {
            if (planes[i].distance(point) < 0) return false;
        }
        return true;
    }

    // Проверка AABB (например, чанк)
    bool boxInFrustum(const glm::vec3& min, const glm::vec3& max) const {
        for (int i = 0; i < 6; i++) {
            glm::vec3 p = min;
            if (planes[i].normal.x >= 0) p.x = max.x;
            if (planes[i].normal.y >= 0) p.y = max.y;
            if (planes[i].normal.z >= 0) p.z = max.z;
            if (planes[i].distance(p) < -1e-4f) return false;
        }
        return true;
    }
};

#endif