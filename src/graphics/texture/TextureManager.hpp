#ifndef TEXTUREMANAGER_HPP
#define TEXTUREMANAGER_HPP

#include <memory>
#include <string>
#include <unordered_map>

#include "Texture.hpp"

class TextureManager {
    std::unordered_map<std::string, std::unique_ptr<Texture>> textures;
};

#endif