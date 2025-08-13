#ifndef TEXTUREIO_HPP
#define TEXTUREIO_HPP

#include <string>

struct Texture;

class TextureIO {
    /*
     * @brief Takes .png image and loads to memory.
     */
    Texture* loadTexture(std::string filename);

    /*
     * @brief saves texture into .png image.
     * @return success
    */
    bool saveTexture(std::string filename, Texture* texture);
};

#endif