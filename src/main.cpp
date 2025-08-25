#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>	

#include "glm/ext/vector_double3.hpp"
#include "graphics/Camera.hpp"
#include "graphics/Frustum.hpp"
#include "graphics/model/Mesh.hpp"
#include "graphics/model/Texture.hpp"
#include "graphics/Shader.hpp"
#include "graphics/renderer/BlockRenderer.hpp"
#include "graphics/renderer/DrawContext.hpp"

#include "blocks/Block.hpp"
#include "blocks/Chunk.hpp"
#include "blocks/Chunks.hpp"

#include "graphics/renderer/DrawContext.hpp"
#include "graphics/renderer/Renderer.hpp"
#include "lighting/LightMap.hpp"
#include "lighting/LightSolver.hpp"

#include "window/Window.hpp"
#include "window/Events.hpp"

#include "window/Texture_loader.hpp"

#include "noise/PerlinNoise.hpp"

#include "glm/glm.hpp"
#include "glm/ext.hpp"

#include <chrono>
#include <thread>
#include <filesystem>

int WIDTH = 1920;
int HEIGHT = 1080;

int main(int argc, char* argv[])
{
	std::string absolute_path = std::filesystem::absolute(argv[0]).parent_path().string();

	std::ios::sync_with_stdio(false);
	std::cin.tie(nullptr);

	Window::init(WIDTH, HEIGHT, "Test Window");
	Events::init();

	Shader* shader = load_shader(absolute_path + "\\res\\shaders\\core.vert", absolute_path + "\\res\\shaders\\core.frag");
	if (shader == nullptr) {
		std::cerr << "failed to load shader" << std::endl;
		Window::terminate();
		return 1;
	}

	Texture* texture = load_texture(absolute_path + "\\res\\images\\block.png");
	if (texture == nullptr) {
		std::cerr << "failed to load texture" << std::endl;
		delete texture;
		Window::terminate();
		return 1;
	}

	std::cout << 1 << '\n';

	Chunks* world = new Chunks(5, 1, 5, true);

	std::cout << 2 << '\n';

	//Mesh** meshes = new Mesh * [world->volume];
	//for (size_t i = 0; i < world->volume; i++)
	//	meshes[i] = nullptr;

	glClearColor(0.6f, 0.62f, 0.65f, 1);
	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	std::cout << 5 << '\n';

	std::cout << 6 << '\n';
	
	//IMGUI_CHECKVERSION();
	//ImGui::CreateContext();
	//ImGuiIO& io = ImGui::GetIO(); (void)io;
	//ImGui::StyleColorsDark();
	//ImGui_ImplGlfw_InitForOpenGL(Window::window, true);
	//ImGui_ImplOpenGL3_Init("#version 330");

	Camera* camera = new Camera(glm::dvec3(1, 0, 0), glm::radians(90.0f));
	
	Frustum* frustum = new Frustum;

	float camX = 0.0f;
	float camY = 0.0f;

	double speed = 25;

	double lastTime = glfwGetTime();
	const double target_fps = 165.0;
	const double H = 1.0f / target_fps;

	double timeAccu = 0.0f;

	DrawContext drawContext(new Renderer(camera, shader, frustum));

	drawContext.registerRenderer("world_renderer", new BlockRenderer(world));

	Events::toggle_cursor();
	while (!Window::isShouldClose()) {
		double currentTime = glfwGetTime();
		double frameTime = currentTime - lastTime;
		lastTime = currentTime;
		
		timeAccu += frameTime;
		if (timeAccu >= H) {
			if (Events::jpressed(GLFW_KEY_ESCAPE)) {
				Window::setShouldClose(true);
			}
			if (Events::jpressed(GLFW_KEY_TAB)) {
				Events::toggle_cursor();
			}

			if (Events::pressed(GLFW_KEY_W)) {
				camera->translate(camera->zdir() * H * speed);
			}
			if (Events::pressed(GLFW_KEY_S)) {
				camera->translate(-camera->zdir() * H * speed);
			}
			if (Events::pressed(GLFW_KEY_D)) {
				camera->translate(camera->xdir() * H * speed);
			}
			if (Events::pressed(GLFW_KEY_A)) {
				camera->translate(-camera->xdir() * H * speed);
			}
			if (Events::pressed(GLFW_KEY_SPACE)) {
				camera->setydir(glm::dvec3(0, 1, 0));

				camera->translate(camera->ydir() * H * speed);
			}
			if (Events::pressed(GLFW_KEY_LEFT_SHIFT)) {
				camera->setydir(glm::dvec3(0, 1, 0));

				camera->translate(-camera->ydir() * H * speed);
			}
			if (Events::pressed(GLFW_KEY_0)) {
				camera->set(0, 0, 0);
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

				camera->setRotation(glm::mat4(1.0f));
				camera->rotate(camY, camX, 0);
			}

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			camera->originRebase();

			//ImGui_ImplOpenGL3_NewFrame();
			//ImGui_ImplGlfw_NewFrame();
			//ImGui::NewFrame();
			
			shader->use();

			glm::mat4 projview = camera->getProjection() * camera->getView();
			
			shader->uniformMatrix("projview", projview);
			texture->bind();
			
			world->update(camera->getPosition());

			//frustum->update(projview);
			drawContext.render();

			//ImGui::ShowDemoWindow();

			//ImGui::Render();
			//ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

			Window::swapBuffers();
			Events::pullEvents();

			timeAccu -= H;
		} else {
			double sleepTime = H - timeAccu;
        	std::this_thread::sleep_for(std::chrono::duration<double>(sleepTime));
		}
	}
	//ImGui_ImplOpenGL3_Shutdown();
	//ImGui_ImplGlfw_Shutdown();
	//ImGui::DestroyContext();

	delete texture;
	delete shader;
	delete frustum;

	// delete solverR;
	// delete solverG;
	// delete solverB;
	// delete solverS;

	delete camera;
	delete world;

	Window::terminate();
	return 0;
}