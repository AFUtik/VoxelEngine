#include "Server.hpp"
#include "../client/Client.hpp"

#include "net/ChunkNet.hpp"

void IntergratedServer::onMessage(ClientMessage msg) {
    ClientMessages messageId = static_cast<ClientMessages>(msg.index());
    switch (messageId) {
        case ClientMessages::Input: {

            break;
        }
        case ClientMessages::ChunkSync: {
            ClientChunkSync chunkSync = std::get<ClientChunkSync>(msg);
            ChunkPtr chunk = world.getChunk(chunkSync.x, chunkSync.y, chunkSync.z);

            ServerChunkStateMsg chunkState;
            if (!chunk) {
                world.generateChunk(chunkSync.x, chunkSync.y, chunkSync.z);
                chunkState.data = serializeChunk(world.getChunk(chunkSync.x, chunkSync.y, chunkSync.z));
            }
            else {
                chunkState.data = serializeChunk(chunk);
            }
            client->onMessage(chunkState);
            break;
        }
        default: {
            break;
        }
    }
}

void IntergratedServer::logicUpdate(double dt) {
    //entitySystem.step(dt);
    //const dvec3 &pos = entitySystem.findById(0)->position;
}

void IntergratedServer::logicLoop() {
    using clock = std::chrono::steady_clock;
    constexpr double TICK = 1.0 / 20.0; // 20 Ticks Per Second

    auto last = clock::now();
    double acc = 0.0;

    while (running)
    {
        auto now = clock::now();
        acc += std::chrono::duration<double>(now - last).count();
        last = now;

        while (acc >= TICK)
        {
            logicUpdate(TICK);
            acc -= TICK;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
