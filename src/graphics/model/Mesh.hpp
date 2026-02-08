#ifndef GRAPHICS_MESH_HPP_
#define GRAPHICS_MESH_HPP_

#include <memory>
#include <mutex>
#include <atomic>

#include "../vertex/VertexInfo.hpp"

class GlController;

class Mesh : std::enable_shared_from_this<Mesh> {
	GlController* glController;
	
	friend class GlController;
	friend class ChunkMesher;
public:
	uint32_t VBO, VAO, EBO;
	std::vector<Vertex> buffer;
	std::vector<uint32_t> indices;
	
	std::atomic<bool> uploaded;

	int vertices = 0; 
	void clearBuffers() {
		buffer  = std::vector<Vertex>();
		//indices = std::vector<uint32_t>();

		buffer.reserve(4096);
		//indices.reserve(4096);
	}

	Mesh(GlController* glController);

	~Mesh();

	inline bool isUploaded() const noexcept {
		return uploaded.load(std::memory_order_acquire);
	}

	inline void setUploaded(bool v) {
		uploaded.store(v, std::memory_order_release);
	}

	void update();
	void draw() const;
};

using MeshPtr = std::shared_ptr<Mesh>;

#endif /* GRAPHICS_MESH_H_ */