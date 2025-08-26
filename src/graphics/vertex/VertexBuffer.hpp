#ifndef VERTEXBUFFER_HPP
#define VERTEXBUFFER_HPP

#include "VertexInfo.hpp"
#include <structures/PackedFreelist.hpp>

struct VertexBuffer {
    packed_freelist<Vertex> vertices;

	VertexBuffer() : vertices(64) {}
};

#endif