#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <memory>
#include <atomic>

#include "ServerCommunication.hpp"
#include "logic/World.hpp"

class IntergratedServer;
class Camera;

class Client {
public:
    explicit Client(IntergratedServer* server);
    ~Client();

    void update(double dt);

    void onKey(uint8_t key, bool pressed);
    void onMouseMove(double dx, double dy);

    const World& getLocalWorld() {return localWorld;}
    const dvec3& getLocalPosition() {return localPosition;}
private:
    void pollServer();
    void sendInput();
private:
    IntergratedServer* server;
    World localWorld;
    dvec3 localPosition;

    uint32_t playerId = 0;
};

#endif