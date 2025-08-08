#ifndef VERTEXCONSUMER_HPP
#define VERTEXCONSUMER_HPP

#include "VertexBuffer.hpp"

class VertexConsumer {
	VertexBuffer* buffer;
public:
    unsigned int ebo_index = 0;
	unsigned int ebo_offset = 0;
	unsigned int vbo_index = 0;

    VertexConsumer(VertexBuffer* buffer) : buffer(buffer) {};
	VertexConsumer() : buffer(nullptr) {};

	void vertex(const Vertex &vertex);
	VertexConsumer& index(unsigned int i1, unsigned int i2, unsigned int i3);

    void endIndex();
};

#endif