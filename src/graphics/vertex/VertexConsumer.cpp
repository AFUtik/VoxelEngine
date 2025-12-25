#include "VertexConsumer.hpp"
#include "iostream"

void VertexConsumer::vertex(const Vertex& vertex)
{
	buffer->vec.push_back(vertex);
	index++;
}

void VertexConsumer::replaceLight(float r, float g, float b, float s){
	Vertex& vertex = buffer->vec[index2];
	vertex.lr = r;
	vertex.lg = g;
	vertex.lb = b;
	vertex.ls = s;
	index2++;
}