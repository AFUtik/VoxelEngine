#ifndef VERTEXCONSUMER_HPP
#define VERTEXCONSUMER_HPP

#include "VertexBuffer.hpp"

class VertexConsumer {
	VertexBuffer* buffer;
	uint32_t index = 0, index2 = 0;
public:
    //unsigned int ebo_index = 0;
	//unsigned int vbo_index = 0;

	inline uint32_t getIndex() {return index;}

    VertexConsumer(VertexBuffer* buffer) : buffer(buffer) {};
	VertexConsumer() : buffer(nullptr) {};

	void vertex(const Vertex &vertex);
	
	void replaceLight(float r, float g, float b, float s);
	void replaceUV   (float u, float v);
	void replaceAll  (const Vertex &vertex);

    void endIndex();
};

#endif