#include "Mesh.hpp"
#include <GL/glew.h>

#include "../vertex/VertexInfo.hpp"
#include <iostream>

void Mesh::upload_buffers() {
	vertices = buffer->vertices.size();

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * VERTEX_SIZE * buffer->vertices.size(), buffer->vertices.data(), GL_STATIC_DRAW);

	// attributes
	uint32_t offset = 0;
	uint32_t i = 0;
	while(ATTRIBUTES[i]) {
		const uint32_t &size = ATTRIBUTES[i];
		glVertexAttribPointer(i, size, GL_FLOAT, GL_FALSE, VERTEX_SIZE * sizeof(GLfloat), (GLvoid*)(offset * sizeof(float)));
		glEnableVertexAttribArray(i);
		offset += size;
		i++;
	}

	glGenBuffers(1, &IVBO);
	glBindBuffer(GL_ARRAY_BUFFER, IVBO);
	glBufferData(
		GL_ARRAY_BUFFER,
		instance_buffer_size * INSTANCE_MEMORY_SIZE_F,
		nullptr,
		GL_DYNAMIC_DRAW
	);
	for (int j = 0; j < 4; j++) {
		glEnableVertexAttribArray(j + i);
		glVertexAttribPointer(j + i, 4, GL_FLOAT, GL_FALSE, INSTANCE_MEMORY_SIZE_F, (void*)(sizeof(GLfloat) * 4 * j));
		glVertexAttribDivisor(j + i, 1);
	}
	i+=4;
	glEnableVertexAttribArray(i);
	glVertexAttribPointer(i, 2, GL_FLOAT, GL_FALSE, INSTANCE_MEMORY_SIZE_F, (void*)(sizeof(GLfloat)*16));
	glVertexAttribDivisor(i, 1);

	glBindVertexArray(0);
}

void Mesh::updateInstanceBuffer(unsigned int index, unsigned int offset, const Instance &instance) {
	glBindBuffer(GL_ARRAY_BUFFER, IVBO);
	void* ptr = glMapBufferRange(GL_ARRAY_BUFFER, index * INSTANCE_MEMORY_SIZE_F, INSTANCE_MEMORY_SIZE_F,
								 GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
	if (ptr) {
		memcpy(ptr, &instance, INSTANCE_MEMORY_SIZE_F);
		glUnmapBuffer(GL_ARRAY_BUFFER);
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
Mesh::~Mesh() {
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &IVBO);
}

void Mesh::draw(unsigned int primitive) const {
	glBindVertexArray(VAO);
	glDrawArraysInstanced(GL_TRIANGLES, 0, vertices, instances);
	glBindVertexArray(0);
}