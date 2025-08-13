#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <cstdint>

struct Texture {
    uint8_t* raw;
    int32_t width, height;

    ~Texture() {if(raw) delete raw;}
};

#endif