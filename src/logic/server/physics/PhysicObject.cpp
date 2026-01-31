#include "PhysicObject.hpp"
#include <iostream>

void PhysicObject::applyForce(dvec3 force) {
    acceleration = force / mass;
}

void PhysicObject::update(double dt) {
    position += velocity * dt;
}