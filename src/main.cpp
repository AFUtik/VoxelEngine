#include <iostream>

#define GLEW_STATIC

#include <GL/glew.h>
#include <GLFW/glfw3.h>	

#include "Shader.hpp"
#include "Window.hpp"
#include "Events.hpp"

int WIDTH = 1280;
int HEIGHT = 720;

// The Original Code from https ://github.com/MihailRis/VoxelEngine-Cpp

float vertices[] = {
	0.0f, 0.0f, 0.0f,
	1.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
};

int main()
{
	Window::init(WIDTH, HEIGHT, "Test Window");
	Events::init();

	Shader* shader = load_shader("res/core.vert", "res/core.frag");
	if (shader == nullptr) {
		std::cerr << "failed to load shader" << std::endl;
		Window::terminate();
		return 1;
	}

	//Create VAO, VBO
	GLuint VAO, VBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	// position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)(0 * sizeof(GLfloat)));
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);

	glClearColor(0.6f, 0.62f, 0.65f, 1);

	while (!Window::isShouldClose()) {
		Events::pullEvents();
		if (Events::jpressed(GLFW_KEY_ESCAPE)) {
			Window::setShouldClose(true);
		}
		if (Events::jclicked(GLFW_MOUSE_BUTTON_1)) {
			glClearColor(0.8f, 0.4f, 0.2f, 1);
		}

		glClear(GL_COLOR_BUFFER_BIT);

		shader->use();
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glBindVertexArray(0);

		Window::swapBuffers();
	}
	delete shader;

	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);

	Window::terminate();
	return 0;
}