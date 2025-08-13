#ifndef TEXTURERENDER_HPP
#define TEXTURERENDER_HPP

#include <cstdint>

struct Texture;

struct TextureRender {
    uint32_t id;

    TextureRender(Texture* texture);
    ~TextureRender();

    void bind();
};

#endif