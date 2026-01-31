#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "../Node.hpp"

#include "blocks/ChunkClient.hpp"
#include "../server/ServerCommunication.hpp"
#include "../../graphics/renderer/ChunkMesher.hpp"

#include <ptypes.hpp>

class IntergratedServer;
class Camera;

const double INPUT_DT = 1.0 / 20.0;

enum Buttons : uint8_t {
    Forward = 1 << 0,
    Back    = 1 << 1,
    Left    = 1 << 2,
    Right   = 1 << 3,
    Jump    = 1 << 4,
};

class Client : public Node {
private:
    Mesher* mesher;

    std::unordered_map<Vector3I, std::shared_ptr<ChunkClient>> chunks;
    Vector3I lastPlayerChunk = Vector3I(INT_MIN, 0, INT_MIN);

    IntergratedServer* server;
    Camera* camera;

    Vector3 playerPos = Vector3(0);

    const static int CLIENT_LOADDISTANCE = 3;

    double inputAccumulator = 0;
    size_t tick = 0;
public:
    explicit Client(Mesher* mesher, Camera* camera) : mesher(mesher), camera(camera) {};

    void physicsProcess(double dt) override;

    //void onKey(uint8_t key, bool pressed);
    //void onMouseMove(double dx, double dy);

    friend class BlockRenderer;
    void onMessage(ServerMessage message);

    inline void setServer(IntergratedServer* server) {
        this->server = server;
    }
};

#endif