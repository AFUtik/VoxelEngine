#ifndef GRAPHICS_MESH_HPP_
#define GRAPHICS_MESH_HPP_

#include <memory>

#include "../Instance.hpp"
#include "../vertex/VertexBuffer.hpp"

#include "../vertex/VertexConsumer.hpp"

class Mesh {
	std::unique_ptr<VertexBuffer> buffer;

	uint32_t VBO, VAO, IVBO;
	uint32_t vertices  = 0;
	uint32_t instances = 1;

	uint32_t instance_buffer_size = 1;
public:
	Mesh(VertexBuffer* buffer) : buffer(buffer) {}
	Mesh() : buffer(new VertexBuffer) {}
	~Mesh();

	inline VertexConsumer getConsumer() const {return VertexConsumer(buffer.get());}

	void upload_buffers();

	void draw(unsigned int primitive) const;

	void updateInstanceBuffer(unsigned int index, unsigned int offset, const Instance& instance);

	void updateVBO(unsigned int offset, unsigned int amount);
};

#endif /* GRAPHICS_MESH_H_ */