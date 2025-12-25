#ifndef VERTEXCONSUMER_HPP
#define VERTEXCONSUMER_HPP

#include "VertexInfo.hpp"

class VertexConsumer {
	VertexBuffer* buffer;
	uint32_t index = 0, index2 = 0;
public:
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