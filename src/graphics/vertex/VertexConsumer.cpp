#include "VertexConsumer.hpp"
#include "VertexBuffer.hpp"
#include "iostream"

void VertexConsumer::vertex(const Vertex& vertex){
	buffer->vertices.allocate(vertex);
	vbo_index += VERTEX_SIZE;
}