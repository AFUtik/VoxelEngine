#ifndef SERVERCOMMANDS_HPP
#define SERVERCOMMANDS_HPP

#include <cstdint>
#include <glm/glm.hpp>
#include <variant>

#include "blocks/Chunk.hpp"

using namespace std;
using namespace glm;

class IntegratedServer;

// CLIENT MESSAGES //

enum class ClientMessages {
    Input,
    ChunkSync
};

struct ClientInputMsg {
    uint32_t tick;
    uint8_t buttons;
    dvec3 forward;
};

struct ClientChunkSync {
    int32_t x, y, z;
    uint32_t version;
};

using ClientMessage = std::variant<ClientInputMsg, ClientChunkSync>;

// SERVER MESSAGES //

enum class ServerMessages {
    PlayerState,
    ChunkState
};

struct ServerPlayerStateMsg {
    uint32_t tick;
    dvec3 pos;
    dvec3 vel;
};

struct ServerChunkStateMsg {
    std::vector<uint8_t> data;

    ServerChunkStateMsg() = default;
};

using ServerMessage = std::variant<ServerPlayerStateMsg, ServerChunkStateMsg>;

#endif