#include "VertexConsumer.hpp"
#include "VertexBuffer.hpp"
#include "iostream"

void VertexConsumer::vertex(const Vertex& vertex)
{
	buffer->vertices.push(vertex, index);
	index++;
}

void VertexConsumer::replaceLight(float r, float g, float b, float s){
	Vertex& vertex = buffer->vertices[index2];
	vertex.lr = r;
	vertex.lg = g;
	vertex.lb = b;
	vertex.ls = s;
	index2++;
}