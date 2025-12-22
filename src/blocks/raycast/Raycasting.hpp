#include <glm/glm.hpp>
#include <memory>

#include "../Block.hpp"

class Chunk;
class Chunks;

struct BlockHit {
    bool hit = false;
    int lx, ly, lz;
    std::shared_ptr<Chunk> chunk;
    block blockData;
};

extern BlockHit raycastBlock(const glm::dvec3& playerPos,
                      const glm::vec3& viewDir,
                      float maxDistance,
                      Chunks* world);