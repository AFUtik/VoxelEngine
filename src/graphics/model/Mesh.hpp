#ifndef GRAPHICS_MESH_HPP_
#define GRAPHICS_MESH_HPP_

#include <memory>

#include "../Instance.hpp"
#include "../vertex/VertexBuffer.hpp"

#include "../vertex/VertexConsumer.hpp"

class Mesh {
	std::unique_ptr<VertexBuffer> buffer;

	uint32_t VBO, VAO, EBO, IVBO;
	uint32_t indices   = 0;
	uint32_t instances = 1;

	uint32_t instance_buffer_size = 1;
public:
	Mesh(VertexBuffer* buffer) : buffer(buffer) {}
	Mesh() : buffer(new VertexBuffer) {}
	~Mesh();

	inline VertexConsumer get_consumer() const {return VertexConsumer(buffer.get());}

	void upload_buffers();

	void draw(unsigned int primitive) const;

	void updateInstanceBuffer(unsigned int index, unsigned int offset, const Instance& instance);
};

#endif /* GRAPHICS_MESH_H_ */