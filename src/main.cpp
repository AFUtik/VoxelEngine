#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>	

#include "glm/ext/vector_double3.hpp"
#include "graphics/Camera.hpp"
#include "graphics/model/Texture.hpp"
#include "graphics/Shader.hpp"
#include "graphics/renderer/BlockRenderer.hpp"
#include "graphics/renderer/DrawContext.hpp"

#include "logic/blocks/AbstractBlock.hpp"
#include "logic/blocks/BlockRegistry.hpp"
#include "logic/blocks/chunk_utils.hpp"
#include "logic/lighting/LightSolver.hpp"
#include "logic/blocks/raycast/Raycasting.hpp"
#include "logic/World.hpp"

#include "graphics/renderer/DrawContext.hpp"
#include "graphics/renderer/Renderer.hpp"

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
	std::cout << "+x" << " " << neighbourIndexFromDelta(1, 0, 0) << std::endl;
	std::cout << "-x" << " " << neighbourIndexFromDelta(-1, 0, 0) << std::endl;
	std::cout << "+y" << " " << neighbourIndexFromDelta(0, 1, 0) << std::endl;
	std::cout << "-y" << " " << neighbourIndexFromDelta(0, -1, 0) << std::endl;
	std::cout << "+z" << " " << neighbourIndexFromDelta(0, 0, 1) << std::endl;
	std::cout << "-z" << " " << neighbourIndexFromDelta(0, 0, -1) << std::endl;

	BlockModel model;
	model.id = 1;
	model.fullCube = true;
	for(int i = 0; i < 6; i++) model.faces[i].texture = 1;
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
	
	World* logic = new World();

	uptr<Camera> camera   = std::make_unique<Camera>(glm::dvec3(0, 0, 0), glm::radians(90.0f));
	uptr<Frustum> frustum = std::make_unique<Frustum>(camera.get());

	DrawContext drawContext(new Renderer(camera.get(), shader.get(), frustum.get()));
	drawContext.registerRenderer("world_renderer", new BlockRenderer(logic));

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
			if (Events::jpressed(GLFW_KEY_ESCAPE)) {
				Window::setShouldClose(true);
			}
			if (Events::jpressed(GLFW_KEY_TAB)) {
				Events::toggle_cursor();
			}

			if (Events::pressed(GLFW_KEY_W)) {
				logic->enqueuePlayerMove(PlayerMoveInput{camera->zdir()});
			}
			if (Events::pressed(GLFW_KEY_S)) {
				logic->enqueuePlayerMove(PlayerMoveInput{-camera->zdir()});
			}
			if (Events::pressed(GLFW_KEY_D)) {
				logic->enqueuePlayerMove(PlayerMoveInput{camera->xdir()});
			}
			if (Events::pressed(GLFW_KEY_A)) {
				logic->enqueuePlayerMove(PlayerMoveInput{-camera->xdir()});
			}
			if (Events::jpressed(GLFW_KEY_H)) {
				glm::ivec3 pos = worldToChunk3(camera->getPosition());
				auto chunk = logic->getChunk(pos.x, pos.y, pos.z);
			}
			if (Events::jpressed(GLFW_KEY_J)) {
				BlockHit hit = raycastBlock(camera->getPosition(), camera->getViewDir(), 15.5, logic);
				if(hit.hit) logic->enqueueCommand([logic=logic, hit] {
					AbstractBlock block;
					block.id = 0;
					block.emission = {0, 0, 15, 15};

					logic->placeBlock(hit.x, hit.y + 1, hit.z, block);
				});
			}
			if (Events::pressed(GLFW_KEY_SPACE)) {
				camera->setydir(glm::dvec3(0, 1, 0));

				logic->enqueuePlayerMove(PlayerMoveInput{camera->ydir()});
			}
			if (Events::pressed(GLFW_KEY_LEFT_SHIFT)) {
				camera->setydir(glm::dvec3(0, 1, 0));

				logic->enqueuePlayerMove(PlayerMoveInput{-camera->ydir()});
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
			camera->set(logic->getPlayerPos());

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			camera->originRebase();
			shader->use();

			glm::mat4 projview = camera->getProjection() * camera->updateView();
			
			shader->uniformMatrix("projview", projview);
			texture->bind();

			frustum->update(projview);
			drawContext.render();

			Window::swapBuffers();
			Events::pullEvents();

			timeAccu -= H;
		} else {
			double sleepTime = H - timeAccu;
        	std::this_thread::sleep_for(std::chrono::duration<double>(sleepTime));
		}
	}

	delete logic;

	Window::terminate();
	return 0;
}