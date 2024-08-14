#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <string>

class Texture {
public:
	unsigned int id;
	Texture(unsigned int id);
	~Texture();

	void bind();
};

#endif // !TEXTURE_HPP
