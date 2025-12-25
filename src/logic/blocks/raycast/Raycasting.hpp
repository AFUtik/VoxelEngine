#include <glm/glm.hpp>

class Chunk;
class LogicSystem;

struct BlockHit {
    bool hit = false;
    int x, y, z;
};

extern BlockHit raycastBlock(const glm::dvec3& playerPos,
                      const glm::vec3& viewDir,
                      float maxDistance,
                      LogicSystem* world);