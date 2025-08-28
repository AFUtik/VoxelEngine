#ifndef GRAPHICS_MESH_HPP_
#define GRAPHICS_MESH_HPP_

#include <memory>

#include "../Instance.hpp"
#include "../vertex/VertexBuffer.hpp"

#include "../vertex/VertexConsumer.hpp"

class Mesh {
	std::shared_ptr<VertexBuffer> buffer;

	bool uploaded = false;

	uint32_t VBO, VAO;
	uint32_t vertices  = 0;
public:
	Mesh(VertexBuffer* buffer) : buffer(buffer) {}
	Mesh() : buffer(std::make_shared<VertexBuffer>()) {}
	
	~Mesh();

	inline bool isUploaded() {return uploaded;}

	inline VertexConsumer getConsumer() const {return VertexConsumer(buffer.get());}

	void upload_buffers();

	void draw(unsigned int primitive) const;
	
	void updateVBO(unsigned int offset, unsigned int amount);
};

#endif /* GRAPHICS_MESH_H_ */