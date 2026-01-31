#ifndef ENTITY_HPP
#define ENTITY_HPP

#include <string>
#include "PhysicObject.hpp"

class Entity : public PhysicObject {
public:     
    unsigned int id = 0;
    std::string name = "";

    Entity() : PhysicObject({0, 128, 0}, 25) {}
};

#endif