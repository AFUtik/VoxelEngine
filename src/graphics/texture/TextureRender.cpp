#include "TextureRender.hpp"
#include "Texture.hpp"

#include "GL/glew.h"
#include <cmath>
#include <string.h>
#include <iostream>

TextureRender::TextureRender(Texture* texture) {
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->width, texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)texture->raw);

    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);

    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    //
    //GLfloat maxAniso = 1.0f;
    //glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
//
    //std::cout << maxAniso << std::endl;
    //glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
    //glTexParameterf(
    //    GL_TEXTURE_2D,
    //    GL_TEXTURE_MAX_ANISOTROPY_EXT,
    //    std::fminf(16.0f, maxAniso)
    //);
    
    glBindTexture(GL_TEXTURE_2D, 0);
}

TextureRender::~TextureRender() {
    glDeleteTextures(1, &id);
}

void TextureRender::bind() {
 	glBindTexture(GL_TEXTURE_2D, id);
}

