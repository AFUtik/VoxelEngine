#ifndef VERTEXBUFFER_HPP
#define VERTEXBUFFER_HPP

#include "VertexInfo.hpp"
#include <cstdint>
#include <vector>

struct VertexBuffer {
    std::vector<Vertex> vertices;

	VertexBuffer(const uint32_t &vertices_size)
	{
		vertices.resize(vertices_size);
	}
	VertexBuffer() {}
};

#endif