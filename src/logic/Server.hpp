#ifndef SERVER_HPP
#define SERVER_HPP

#include "World.hpp"
#include "physics/EntitySystem.hpp"

#include "ServerCommunication.hpp"

#include <deque>
#include <mutex>

class IntergratedServer { 
private:
    World world;
    EntitySystem entitySystem;
    

    std::deque<ServerCommand> commands;
    std::mutex commandMutex;

    std::deque<ServerMessage> out;
    std::mutex messageMutex;

    std::thread logicThread;
    std::atomic<bool> running{true};

    void sendToClient();
    
    void handleCommand(ServerCommand &cmd);
public:
    IntergratedServer() : entitySystem(&world) {
        logicThread = std::thread([this] { logicLoop(); });

        auto player = std::make_unique<Entity>();
        player->id = 0;

        entitySystem.create(std::move(player));
    }

    ~IntergratedServer() {
        running = false;
        if (logicThread.joinable())
            logicThread.join();
    }

    ServerMessage receiveMessage();
    void pushCommand(ServerCommand &cmd);

    void logicUpdate(double dt);
    void logicLoop();
    
    //void logicFreeze();
};

#endif