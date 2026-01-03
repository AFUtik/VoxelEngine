#include "Client.hpp"

void Client::sendInput() {
    InputCmd cmd;
    cmd.tick = tick++;
    cmd.buttons = buttons.load();
    cmd.forward = forward;

    server.pushInput(cmd);
}