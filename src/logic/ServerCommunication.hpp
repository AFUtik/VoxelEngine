#ifndef SERVERCOMMANDS_HPP
#define SERVERCOMMANDS_HPP

#include <cstdint>
#include <glm/glm.hpp>

#include <variant>

using namespace std;
using namespace glm;

class IntegratedServer;

// CMD

struct InputCmd {
    uint32_t tick;
    uint8_t buttons;
    dvec3 forward;
};

using ServerCommand = std::variant<InputCmd>;

enum class ServerCommands {
    InputCmd
};

// MESSAGE

struct PlayerState {
    uint32_t tick;
    dvec3 pos;
    dvec3 vel;
};

using ServerMessage = std::variant<std::monostate, PlayerState>;

enum class ServerMessages {
    Empty,
    PlayerState
};

#endif