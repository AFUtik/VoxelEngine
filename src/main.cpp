#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>	

#include "Camera.hpp"
#include "Mesh.hpp"

#include "Shader.hpp"
#include "Window.hpp"
#include "Events.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "Texture.hpp"
#include "Texture_loader.hpp"

#include "glm/glm.hpp"
#include "glm/ext.hpp"

int WIDTH = 1920;
int HEIGHT = 1080;

//Original Code from https://github.com/MihailRis/VoxelEngine-Cpp

float vertices[] = {
	-1.0f,-1.0f, 0.0f, 0.0f, 0.0f,
	1.0f,-1.0f, 0.0f, 1.0f, 0.0f,
	-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,

	1.0f,-1.0f, 0.0f, 1.0f, 0.0f,
	1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
	  -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
};

int attrs[]{
	3, 2, 0 //null terminator
};

int main()
{
	Window::init(WIDTH, HEIGHT, "Test Window");
	Events::init();

	Shader* shader = load_shader("res/shaders/core.vert", "res/shaders/core.frag");
	if (shader == nullptr) {
		std::cerr << "failed to load shader" << std::endl;
		Window::terminate();
		return 1;
	}

	Texture* texture = load_texture("E:/Cpp/VoxelEngine/res/images/new.png");
	if (texture == nullptr) {
		std::cerr << "failed to load texture" << std::endl;
		delete texture;
		Window::terminate();
		return 1;
	}

	Mesh* mesh = new Mesh(vertices, 6, attrs);

	glClearColor(0.6f, 0.62f, 0.65f, 1);

	//IMGUI_CHECKVERSION();
	//ImGui::CreateContext();
	//ImGuiIO& io = ImGui::GetIO(); (void)io;
	//ImGui::StyleColorsDark();
	//ImGui_ImplGlfw_InitForOpenGL(Window::window, true);
	//ImGui_ImplOpenGL3_Init("#version 330");

	Camera* camera = new Camera(glm::vec3(0, 0, 1), glm::radians(90.0f));
	glm::mat4 model(1.0f);

	float lastTime = glfwGetTime();
	float delta = 0.0f;

	float camX = 0.0f;
	float camY = 0.0f;

	float speed = 5;

	Events::toggle_cursor();
	while (!Window::isShouldClose()) {
		float currentTime = glfwGetTime();
		delta = currentTime - lastTime;
		lastTime = currentTime;

		if (Events::jpressed(GLFW_KEY_ESCAPE)) {
			Window::setShouldClose(true);
		}
		if (Events::jpressed(GLFW_KEY_TAB)) {
			Events::toggle_cursor();
		}

		if (Events::pressed(GLFW_KEY_W)) {
			camera->translate_z(delta * speed);
		}
		if (Events::pressed(GLFW_KEY_S)) {
			camera->translate_z(-delta * speed);
		}
		if (Events::pressed(GLFW_KEY_D)) {
			camera->translate_x(delta * speed);
		}
		if (Events::pressed(GLFW_KEY_A)) {
			camera->translate_x(-delta * speed);
		}
		if (Events::pressed(GLFW_KEY_SPACE)) {
			camera->translate_y(delta * speed);
		}
		if (Events::pressed(GLFW_KEY_LEFT_SHIFT)) {
			camera->translate_y(-delta * speed);
		}
		if (Events::pressed(GLFW_KEY_0)) {
			camera->set_xyz(0, 0, 1);
		}

		if (Events::_cursor_locked) {
			camY += -Events::deltaY / Window::height * 2;
			camX += -Events::deltaX / Window::height * 2;

			if (camY < -radians(89.0f)) {
				camY = -radians(89.0f);
			}
			if (camY > radians(89.0f)) {
				camY = radians(89.0f);
			}

			camera->rotation = mat4(1.0f);
			camera->rotate(camY, camX, 0);
		}

		glClear(GL_COLOR_BUFFER_BIT);

		//ImGui_ImplOpenGL3_NewFrame();
		//ImGui_ImplGlfw_NewFrame();
		//ImGui::NewFrame();
		
		shader->use();
		shader->uniformMatrix("model", model);
		shader->uniformMatrix("projview", camera->getProjection() * camera->getView());
		texture->bind();

		mesh->draw(GL_TRIANGLES);

		//ImGui::ShowDemoWindow();

		//ImGui::Render();
		//ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		Window::swapBuffers();
		Events::pullEvents();
	}
	//ImGui_ImplOpenGL3_Shutdown();
	//ImGui_ImplGlfw_Shutdown();
	//ImGui::DestroyContext();
	delete texture;
	delete shader;

	delete mesh;

	Window::terminate();
	return 0;
}