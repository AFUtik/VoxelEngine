#ifndef VERTEXBUFFER_HPP
#define VERTEXBUFFER_HPP

#include "VertexInfo.hpp"
#include <cstdint>
#include <vector>

struct VertexBuffer {
    std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	VertexBuffer(const uint32_t &vertices_size, const uint32_t &indices_size)
	{
		vertices.resize(vertices_size);
		indices.resize(indices_size);
	}
	VertexBuffer() {}
};

#endif