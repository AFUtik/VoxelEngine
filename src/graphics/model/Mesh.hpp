#ifndef GRAPHICS_MESH_HPP_
#define GRAPHICS_MESH_HPP_

#include <memory>
#include <mutex>

#include "../vertex/VertexInfo.hpp"

class GlController;

class Mesh {
	GlController* glContoller;
	
	friend class GlController;
	friend class ChunkMesher;
public:
	uint32_t VBO, VAO, EBO;
	std::vector<Vertex> buffer;
	std::vector<uint32_t> indices;
	
	bool uploaded = false;
	
	int vertices = 0; 

	void clearBuffers() {
		buffer  = std::vector<Vertex>();
		//indices = std::vector<uint32_t>();

		buffer.reserve(4096);
		//indices.reserve(4096);
	}

	mutable std::mutex mutex;

	Mesh(GlController* glController);

	~Mesh();

	inline bool isUploaded() {return uploaded;}

	void update();

	void draw(unsigned int primitive) const;
};

#endif /* GRAPHICS_MESH_H_ */