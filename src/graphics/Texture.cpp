#include "Texture.hpp"

#include "GL/glew.h"

Texture::Texture(unsigned int id) : id(id) {}

Texture::~Texture() {
	glDeleteTextures(1, &id);
}

void Texture::bind() {
	glBindTexture(GL_TEXTURE_2D, id);
}