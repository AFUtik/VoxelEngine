#include "BlockRenderer.hpp"

#include "../Frustum.hpp"
#include "../../logic/client/Client.hpp"

#include "../../graphics/Transform.hpp"

#include <glm/ext.hpp>
#include <memory>
#include <mutex>
#include <shared_mutex>

void BlockRenderer::render() {
    for (const auto& [pos, chunk] : client->chunks) {
        if (!chunk) continue;

        chunk->meshInstance.draw(camera);
    }
}
