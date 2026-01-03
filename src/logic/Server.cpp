#include "Server.hpp"
#include "logic/ServerCommunication.hpp"


void IntergratedServer::handleCommand(ServerCommand &cmd) {
    ServerCommands messageId = static_cast<ServerCommands>(cmd.index());
    switch (messageId) {
        case ServerCommands::InputCmd: {

            break;
        }
        default: {
            break;
        }
    }
}

ServerMessage IntergratedServer::receiveMessage() {
    ServerMessage message;
    {
        std::lock_guard lock(messageMutex);
        if(!out.empty()) {
            message = out.front(); out.pop_front();
        }
        
    }
    return message;
}

void IntergratedServer::pushCommand(ServerCommand &cmd) {
    {
        std::lock_guard lock(commandMutex);
        commands.push_back(cmd);
    }
}

void IntergratedServer::logicUpdate(double dt) {
    entitySystem.step(dt);

    const dvec3 &pos = entitySystem.findById(0)->position;
    world.load(pos.x, pos.y, pos.z);
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
