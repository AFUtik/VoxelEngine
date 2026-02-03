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
            std::cout << "received" << std::endl;

            ChunkClientPtr chunk = deserializeChunkToClient(chunkState.data);
            loadNeighbours(chunk);

            mesher->queueBuildMesh(chunk);
            chunks.emplace(chunk->pos, chunk);
            break;
        }
        default: {
            break;
        }
    }
}

void Client::loadNeighbours(ChunkClientPtr chunk) {
    for (int i = 0; i < 26; ++i) {
        int nx = chunk->pos.x + OFFSETS[i][0];
        int ny = chunk->pos.y + OFFSETS[i][1];
        int nz = chunk->pos.z + OFFSETS[i][2];

        auto it = chunks.find(Vector3I{ nx, ny, nz });
        if (it != chunks.end()) {
            auto& neigh = it->second;
            chunk->loadNeighbour(i, neigh);
        }
    }
}

void Client::physicsProcess(double dt) {
    server->pos = camera->getPosition();

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

    Vector3I playerChunk = { floorDiv(server->pos.x, ChunkInfo::WIDTH), floorDiv(server->pos.y, ChunkInfo::HEIGHT), floorDiv(server->pos.z, ChunkInfo::DEPTH) };

    if (Events::jpressed(GLFW_KEY_B)) {
        chunks.clear();
    }

    if (Events::jpressed(GLFW_KEY_V)) {
        size_t count = 0;
        for (auto& [pos, chunk] : chunks) {
            count += chunk->meshInstance.mesh->vertices;
        }
        std::cout << count << std::endl;
    }

    if (playerChunk != lastPlayerChunk || Events::jpressed(GLFW_KEY_F)) {
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
                if(chunks.find({x, 0, z}) == chunks.end()) server->sendMessage(
                    ClientChunkSync{ x, 0, z, 0 }
                );
            }
        }
        lastPlayerChunk = playerChunk;
    }
}