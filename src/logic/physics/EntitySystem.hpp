#ifndef ENTITYSYSTEM_HPP
#define ENTITYSYSTEM_HPP

#include "Entity.hpp"
#include <map>
#include <unordered_map>
#include <memory>

class World;

class EntitySystem {
    std::vector<std::unique_ptr<Entity>> vec;
    std::unordered_map<std::string, int> str_index;

    World *collision;
public:
    EntitySystem(World *world) : collision(world) {};

    void create(std::unique_ptr<Entity> new_entity) {
        new_entity->id = vec.size();
        vec.push_back(std::move(new_entity));
    }

    Entity* findById(int id) {return vec[id].get();}

    void step(double dt) {
        for(auto &ent_ptr : vec) ent_ptr->update(dt);
    }
};

#endif