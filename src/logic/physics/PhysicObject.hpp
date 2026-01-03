#ifndef PHYSICOBJECT_HPP
#define PHYSICOBJECT_HPP

#include "AABB.hpp"
#include "../blocks/Chunk.hpp"

class PhysicObject {
public:
    cld::AABB collider;
    dvec3 position {0};
    dvec3 velocity {0};
    dvec3 acceleration {0};
    double mass;

    static constexpr double HX = 0.3;
    static constexpr double HY = 0.9;
    static constexpr double HZ = 0.3;

    inline double minX() const { return position.x - HX; }
    inline double maxX() const { return position.x + HX; }
    inline double minY() const { return position.y - HY; }
    inline double maxY() const { return position.y + HY; }
    inline double minZ() const { return position.z - HZ; }
    inline double maxZ() const { return position.z + HZ; }

    bool onGround = false;

    PhysicObject(dvec3 pos, double mass) : position(pos), mass(mass), collider(dvec3(0), dvec3(0)) {
    }

    void applyForce(dvec3 force);

    void update(double dt);
};

#endif