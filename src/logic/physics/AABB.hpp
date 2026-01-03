/**
* @file AABB.hpp
* @brief Axis-Aligned Bounding Box Header file.
* @author AFUtik
* @date 2025-05-26
* @version v1.0.0
* @copyright MIT
*/

#ifndef AABB_HPP
#define AABB_HPP

#include <glm/glm.hpp>
#include <cmath>
using namespace glm;

namespace cld {

/**
* @brief Axis-Aligned Bounding Box
*/
struct AABB {
protected:
    
public:
    dvec3 half;
    dvec3 min_, max_;
    AABB(const dvec3& min, const dvec3& max) : min_(min), max_(max), half(0.4, 1.0, 0.4)
    {

    }

    AABB(float x1, float y1, float z1, float x2, float y2, float z2) :
        min_(x1, y1, z1), max_(x2, y2, z2)
    {

    }

    AABB() = default;

    inline void transform(const dvec3 &position) {
        min_ = position - half;
        max_ = position + half;
    }

    inline const dvec3& min() const { return min_; }
    inline const dvec3& max() const { return max_; }

    inline void setMin(const vec3& min) {min_ = min;}
    inline void setMax(const vec3& max) {max_ = max;}

    inline double getX1() const {return min_.x;}
    inline double getY1() const {return min_.y;}
    inline double getZ1() const {return max_.z;}
    inline double getX2() const {return max_.x;}
    inline double getY2() const {return max_.y;}
    inline double getZ2() const {return max_.z;}

    inline void setX1(float value) {min_.x = value;}
    inline void setY1(float value) {min_.y = value;}
    inline void setZ1(float value) {min_.z = value;}
    inline void setX2(float value) {max_.x = value;}
    inline void setY2(float value) {max_.y = value;}
    inline void setZ2(float value) {max_.z = value;}

    inline bool intersect(const AABB &other) const {
        return (max_.x >= other.min_.x && min_.x <= other.max_.x) &&
               (max_.y >= other.min_.y && min_.y <= other.max_.y) &&
               (max_.z >= other.min_.z && min_.z <= other.max_.z);
        
    }

    inline void expand(const AABB& other)
    {
        min_.x = std::fminf(min_.x, other.min_.x);
        max_.x = std::fmaxf(max_.x, other.max_.x);
        min_.y = std::fminf(min_.y, other.min_.y);
        max_.y = std::fmaxf(max_.y, other.max_.y);
        min_.z = std::fminf(min_.z, other.min_.z);
        max_.z = std::fmaxf(max_.z, other.max_.z);
    }

    inline dvec3 getMin() const {return min_;}
    inline dvec3 getMax() const {return max_;}
};

}

#endif