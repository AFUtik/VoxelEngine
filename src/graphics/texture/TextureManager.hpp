#ifndef TEXTUREMANAGER_HPP
#define TEXTUREMANAGER_HPP

#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

#include "Texture.hpp"
#include "../tilemap/TilemapManager.hpp"

class TextureManager {
    using TexPtr     = std::unique_ptr<Texture>;
    using TilemapPtr = std::unique_ptr<Texture>;

    TilemapManager tilemapManager;

    std::vector<TexPtr> textures;
    std::unordered_map<std::string, uint32_t> textureMap;
public:    
    TilemapManager& getTilemapManager() {return tilemapManager;}

    void loadTextureWithTilemap(
        std::string_view name, 
        const std::filesystem::path& path,
        std::string_view tilemap_name) 
    {

    }

    void loadTexture(
        std::string_view name, 
        const std::filesystem::path& path) 
    {

    }
};

#endif