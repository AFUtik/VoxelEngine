#ifndef SHADER_HPP
#define SHADER_HPP

class Shader {
public:
	unsigned int id;
	Shader(unsigned int id);
	~Shader();

	void use();
};

extern Shader* load_shader();

#endif