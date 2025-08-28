#include "Mesh.hpp"
#include <GL/glew.h>

#include "../vertex/VertexInfo.hpp"
#include <iostream>

void Mesh::upload_buffers() {
	vertices = buffer->vertices.get_size();
	

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * VERTEX_SIZE * buffer->vertices.get_size(), buffer->vertices.get_data(), GL_STATIC_DRAW);

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
	glBindVertexArray(0);
	
	uploaded = true;
}

void Mesh::updateVBO(unsigned int offset, unsigned int amount) {
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferSubData(GL_ARRAY_BUFFER, offset * sizeof(Vertex), amount * sizeof(Vertex), buffer->vertices.get_data() + offset);
}

Mesh::~Mesh() {
	if(uploaded) {
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
	}
}

void Mesh::draw(unsigned int primitive) const {
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, vertices);
	glBindVertexArray(0);
}