#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>	

#include "glm/ext/vector_double3.hpp"
#include "graphics/Camera.hpp"
#include "graphics/model/Texture.hpp"
#include "graphics/Shader.hpp"
#include "graphics/renderer/BlockRenderer.hpp"
#include "graphics/renderer/DrawContext.hpp"

#include "graphics/renderer/DrawContext.hpp"
#include "graphics/renderer/Renderer.hpp"

#include "logic/client/Client.hpp"
#include "logic/server/Server.hpp"

#include "window/Window.hpp"
#include "window/Events.hpp"

#include "loaders/Texture_loader.hpp"

#include <chrono>
#include <memory>
#include <regex>
#include <thread>
#include <filesystem>

int WIDTH = 1920;
int HEIGHT = 1080;

template<typename T> using uptr = std::unique_ptr<T>;

int main(int argc, char* argv[])
{
	std::cout << ChunkClient::INDEX(0, 0, 1) << " " << ChunkClient::INDEX(0, 0, -1) << std::endl;

	BlockModel model;
	model.id = 0;
	model.fullCube = true;
	for(int i = 0; i < 6; i++) model.faces[i].texture = 0;
	BlockModelRegistry::registerBlockModel(model);

	std::string absolute_path = std::filesystem::absolute(argv[0]).parent_path().string();

	std::ios::sync_with_stdio(false);
	std::cin.tie(nullptr);

	Window::init(WIDTH, HEIGHT, "Test Window");
	Events::init();

	uptr<Shader> shader = uptr<Shader>(load_shader(absolute_path + "\\res\\shaders\\core.vert", absolute_path + "\\res\\shaders\\core.frag"));
	if (shader == nullptr) {
		std::cerr << "failed to load shader" << std::endl;
		Window::terminate();
		return 1;
	}
	uptr<Texture> texture = uptr<Texture>(load_texture(absolute_path + "\\res\\images\\atlas.png"));
	if (texture == nullptr) {
		std::cerr << "failed to load texture" << std::endl;
		Window::terminate();
		return 1;
	}
	
	glClearColor(0.6f, 0.62f, 0.65f, 1);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	uptr<GlController> glController = std::make_unique<GlController>();
	uptr<Camera> camera   = std::make_unique<Camera>(glm::dvec3(0, 0, 0), glm::radians(90.0f));

	uptr<ThreadPool> threadPool = std::make_unique<ThreadPool>(4);
	uptr<Mesher> mesher = std::make_unique<Mesher>(glController.get(), shader.get(), threadPool.get());

	uptr<IntergratedServer> server = std::make_unique<IntergratedServer>(threadPool.get());
	uptr<Client> client = std::make_unique<Client>(mesher.get(), camera.get());

	server->setClient(client.get());
	client->setServer(server.get());
		
	DrawContext drawContext(new Renderer(client.get(), camera.get()));
	drawContext.registerRenderer("world_renderer", new BlockRenderer());

	float camX = 0.0f;
	float camY = 0.0f;

	double speed = 25;

	double lastTime = glfwGetTime();
	const double target_fps = 165.0;
	const double H = 1.0f / target_fps;

	double timeAccu = 0.0f;

	Events::toggle_cursor();
	while (!Window::isShouldClose()) {
		double currentTime = glfwGetTime();
		double frameTime = currentTime - lastTime;
		lastTime = currentTime;
		
		timeAccu += frameTime;
		if (timeAccu >= H) {
			glController->processAll();

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

			if (Events::jpressed(GLFW_KEY_ESCAPE)) {
				Window::setShouldClose(true);
			}
			if (Events::jpressed(GLFW_KEY_TAB)) {
				Events::toggle_cursor();
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

			client->physicsProcess(H);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			camera->update();
			shader->use();
			
			shader->uniformMatrix("projview", camera->getProjview());
			texture->bind();

			drawContext.render();

			Window::swapBuffers();
			Events::pullEvents();

			timeAccu -= H;
		} else {
			double sleepTime = H - timeAccu;
        	std::this_thread::sleep_for(std::chrono::duration<double>(sleepTime));
		}
	}
	Window::terminate();
	return 0;
}