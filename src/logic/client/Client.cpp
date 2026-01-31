#include "Client.hpp"

#include "../server/Server.hpp"
#include "../../window/Events.hpp"
#include "../../graphics/Camera.hpp"

#include "../server/net/ChunkNet.hpp"

#include "GLFW/glfw3.h"

void Client::onMessage(ServerMessage msg) {
    ServerMessages messageId = static_cast<ServerMessages>(msg.index());
    switch (messageId) {
        case ServerMessages::PlayerState: {
            
            break;
        }
        case ServerMessages::ChunkState: {
            const auto& chunkState = std::get<ServerChunkStateMsg>(msg);

            ChunkPtr chunk = deserializeChunk(chunkState.data);
            ChunkClientPtr clientChunk = std::make_shared<ChunkClient>(chunk);

            mesher->queueBuildMesh(clientChunk);
            chunks.emplace(chunk->pos, clientChunk);
            break;
        }
        default: {
            break;
        }
    }
}

void Client::physicsProcess(double dt) {
    inputAccumulator += dt;
    if(inputAccumulator >= INPUT_DT) {
        inputAccumulator -= INPUT_DT;
        ClientInputMsg cmd;
        cmd.tick = tick++;
        cmd.buttons = 
            (Events::jpressed(GLFW_KEY_SPACE) << 4) | 
            (Events::jpressed(GLFW_KEY_D) << 3) | 
            (Events::jpressed(GLFW_KEY_A) << 2) | 
            (Events::jpressed(GLFW_KEY_S) << 1) |
             Events::jpressed(GLFW_KEY_W);
        cmd.forward = camera->zdir();
        //server->onMessage(cmd);
    }

    Vector3I playerChunk = { floorDiv(playerPos.x, ChunkInfo::WIDTH), floorDiv(playerPos.y, ChunkInfo::HEIGHT), floorDiv(playerPos.z, ChunkInfo::DEPTH) };

    if (playerChunk != lastPlayerChunk) {
        std::vector<Vector3I> toUnload;
        for (auto& [pos, chunk] : chunks) {
            if (!((abs(playerChunk.x - pos.x) <= CLIENT_LOADDISTANCE) &&
                  (abs(playerChunk.y - pos.y) <= CLIENT_LOADDISTANCE) &&
                  (abs(playerChunk.z - pos.z) <= CLIENT_LOADDISTANCE)))
            {
                toUnload.push_back(pos);
            }
        }
        for (auto pos : toUnload) chunks.erase(pos);

        for (int x = playerChunk.x - CLIENT_LOADDISTANCE; x <= playerChunk.x + CLIENT_LOADDISTANCE; x++) {
            for (int z = playerChunk.z - CLIENT_LOADDISTANCE; z <= playerChunk.z + CLIENT_LOADDISTANCE; z++) {
                server->onMessage(
                    ClientChunkSync{ x, 0, z, 0 }
                );
            }
        }
        lastPlayerChunk = playerChunk;
    }
}