#ifndef GRAPHICS_MESH_HPP_
#define GRAPHICS_MESH_HPP_

#include <memory>

#include "../Instance.hpp"
#include "../vertex/VertexBuffer.hpp"

#include "../vertex/VertexConsumer.hpp"

class GlController;

class Mesh {
	GlController* glContoller;

	std::shared_ptr<VertexBuffer> buffer;

	bool uploaded = false;

	uint32_t VBO, VAO;
	uint32_t vertices  = 0;

	friend class GlController;
public:
	Mesh(VertexBuffer* buffer) : buffer(buffer) {}
	Mesh(GlController* glController);

	~Mesh();

	inline bool isUploaded() {return uploaded;}

	inline VertexConsumer getConsumer() const {return VertexConsumer(buffer.get());}

	void uploadBuffers();

	void draw(unsigned int primitive) const;
};

#endif /* GRAPHICS_MESH_H_ */