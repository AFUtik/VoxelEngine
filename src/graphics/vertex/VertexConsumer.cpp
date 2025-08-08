#include "VertexConsumer.hpp"
#include "VertexBuffer.hpp"
#include "iostream"

void VertexConsumer::vertex(const Vertex& vertex){
	buffer->vertices.push_back(vertex);
	vbo_index += VERTEX_SIZE;
}

VertexConsumer& VertexConsumer::index(unsigned int i1, unsigned int i2, unsigned int i3) {
	buffer->indices.push_back(ebo_offset+i1) ;
	buffer->indices.push_back(ebo_offset+i2);
	buffer->indices.push_back(ebo_offset+i3);
	ebo_index+=3;
	return *this;
}

void VertexConsumer::endIndex() {
	ebo_offset=vbo_index/VERTEX_SIZE;
}