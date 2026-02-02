#ifndef SERVER_HPP
#define SERVER_HPP

#include "../Node.hpp"
#include "World.hpp"
#include "physics/EntitySystem.hpp"
#include "ServerCommunication.hpp"

#include <ptypes.hpp>

class Client;

class IntergratedServer : Node {
private:
    Client* client;

    World world;
    EntitySystem entitySystem;

    std::deque<ClientMessage> in;
    std::mutex inMutex;

    std::deque<ServerMessage> out;
    std::mutex outMutex;

    //std::deque<ServerMessage> out;
    //std::mutex messageMutex;

    std::thread logicThread;
    std::atomic<bool> running{true};

    static const int SERVER_LOADDISTANCE = 3;
public:
    Vector3 pos;

    void onMessage(ClientMessage& cmd);

    IntergratedServer() : world(SERVER_LOADDISTANCE), entitySystem(&world) {
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

    inline void sendMessage(ClientMessage msg) {
        std::lock_guard<std::mutex> lock(inMutex);
        in.push_front(msg);
    }

    inline ServerMessage receiveMessage() {
        std::lock_guard<std::mutex> lock(outMutex);
        ServerMessage msg = out.front(); out.pop_front();
        return msg;
    }

    inline void setClient(Client* client) {
        this->client = client;
    }

    //bool messagesEmpty() {return out.empty();}
    //ServerMessage receiveMessage();
    //void pushCommand(const ServerCommand &cmd);

    void logicUpdate(double dt);
    void logicLoop();
    
    //void logicFreeze();
};

#endif