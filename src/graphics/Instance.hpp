#ifndef INSTANCE_HPP
#define INSTANCE_HPP

#include <glm/glm.hpp>

using Instance = glm::mat4;

#define INSTANCE_DATA_LENGTH 16
#define INSTANCE_MEMORY_SIZE_F (INSTANCE_DATA_LENGTH * sizeof(float))

#endif //INSTANCE_HPP
