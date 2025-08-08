#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>	

#include "graphics/Camera.hpp"
#include "graphics/model/Mesh.hpp"
#include "graphics/model/Texture.hpp"
#include "graphics/Shader.hpp"
#include "graphics/renderer/BlockRenderer.hpp"

#include "blocks/Block.hpp"
#include "blocks/Chunk.hpp"
#include "blocks/Chunks.hpp"

#include "lighting/LightMap.hpp"
#include "lighting/LightSolver.hpp"

#include "window/Window.hpp"
#include "window/Events.hpp"

#include "window/Texture_loader.hpp"

#include "noise/PerlinNoise.hpp"

#include "glm/glm.hpp"
#include "glm/ext.hpp"

int WIDTH = 1920;
int HEIGHT = 1080;

//Original Code from https://github.com/MihailRis/VoxelEngine-Cpp

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

	Texture* texture = load_texture("E:/Cpp/VoxelEngine/res/images/block.png");
	if (texture == nullptr) {
		std::cerr << "failed to load texture" << std::endl;
		delete texture;
		Window::terminate();
		return 1;
	}


	Chunks* chunks = new Chunks(6, 1, 4);
	BlockRenderer renderer(chunks);

	//Mesh** meshes = new Mesh * [chunks->volume];
	//for (size_t i = 0; i < chunks->volume; i++)
	//	meshes[i] = nullptr;

	glClearColor(0.6f, 0.62f, 0.65f, 1);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    /*
	LightSolver* solverR = new LightSolver(chunks, 0);
	LightSolver* solverG = new LightSolver(chunks, 1);
	LightSolver* solverB = new LightSolver(chunks, 2);
	LightSolver* solverS = new LightSolver(chunks, 3);

	for (int y = 0; y < chunks->h * CHUNK_H; y++) {
		for (int z = 0; z < chunks->d * CHUNK_D; z++) {
			for (int x = 0; x < chunks->w * CHUNK_W; x++) {
				block vox = chunks->get(x, y, z);
				if (vox.id == 1) {
					solverR->add(x, y, z, 15);
					solverG->add(x, y, z, 15);
					solverB->add(x, y, z, 15);
				}
			}
		}
	}

	for (int z = 0; z < chunks->d * CHUNK_D; z++) {
		for (int x = 0; x < chunks->w * CHUNK_W; x++) {
			for (int y = chunks->h * CHUNK_H - 1; y >= 0; y--) {
				block vox = chunks->get(x, y, z);
				if (vox.id != 0) {
					break;
				}
				chunks->getChunkByBlock(x, y, z)->lightmap->setS(x % CHUNK_W, y % CHUNK_H, z % CHUNK_D, 0xF);
			}
		}
	}

	for (int z = 0; z < chunks->d * CHUNK_D; z++) {
		for (int x = 0; x < chunks->w * CHUNK_W; x++) {
			for (int y = chunks->h * CHUNK_H - 1; y >= 0; y--) {
				block vox = chunks->get(x, y, z);
				if (vox.id != 0) {
					break;
				}
				if (
					chunks->getLight(x - 1, y, z, 3) == 0 ||
					chunks->getLight(x + 1, y, z, 3) == 0 ||
					chunks->getLight(x, y - 1, z, 3) == 0 ||
					chunks->getLight(x, y + 1, z, 3) == 0 ||
					chunks->getLight(x, y, z - 1, 3) == 0 ||
					chunks->getLight(x, y, z + 1, 3) == 0
					) {
					solverS->add(x, y, z);
				}
				chunks->getChunkByBlock(x, y, z)->lightmap->setS(x % CHUNK_W, y % CHUNK_H, z % CHUNK_D, 0xF);
			}
		}
	}

	solverR->solve();
	solverG->solve();
	solverB->solve();
	solverS->solve();

	*/

	for (size_t i = 0; i < chunks->volume; ++i) renderer.render(chunks->chunks[i]);


	//IMGUI_CHECKVERSION();
	//ImGui::CreateContext();
	//ImGuiIO& io = ImGui::GetIO(); (void)io;
	//ImGui::StyleColorsDark();
	//ImGui_ImplGlfw_InitForOpenGL(Window::window, true);
	//ImGui_ImplOpenGL3_Init("#version 330");

	Camera* camera = new Camera(glm::vec3(0, 0, 1), glm::radians(90.0f));

	float lastTime = glfwGetTime();
	float delta = 0.0f;

	float camX = 0.0f;
	float camY = 0.0f;

	float speed = 25;

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
			camera->position += camera->z_dir * delta * speed;
		}
		if (Events::pressed(GLFW_KEY_S)) {
			camera->position -= camera->z_dir * delta * speed;
		}
		if (Events::pressed(GLFW_KEY_D)) {
			camera->position += camera->x_dir * delta * speed;
		}
		if (Events::pressed(GLFW_KEY_A)) {
			camera->position -= camera->x_dir * delta * speed;
		}
		if (Events::pressed(GLFW_KEY_SPACE)) {
			camera->y_dir = glm::vec3(0, 1, 0);

			camera->position += camera->y_dir * delta * speed;
		}
		if (Events::pressed(GLFW_KEY_LEFT_SHIFT)) {
			camera->y_dir = glm::vec3(0, 1, 0);

			camera->position -= camera->y_dir * delta * speed;
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

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//ImGui_ImplOpenGL3_NewFrame();
		//ImGui_ImplGlfw_NewFrame();
		//ImGui::NewFrame();
		
		shader->use();
		shader->uniformMatrix("projview", camera->getProjection() * camera->getView());
		texture->bind();

		for (size_t i = 0; i < chunks->volume; i++) {
			chunks->chunks[i]->chunk_draw.draw();
		}

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

	// delete solverR;
	// delete solverG;
	// delete solverB;
	// delete solverS;

	delete chunks;

	Window::terminate();
	return 0;
}