#include "Mesh.hpp"
#include <GL/glew.h>

#include "../renderer/GlController.hpp"
#include <mutex>

Mesh::Mesh(GlController* glController) : glContoller(glController), buffer(std::make_shared<VertexBuffer>()) {

}

//void Mesh::updateVBO(unsigned int offset, unsigned int amount) {
//	glBindBuffer(GL_ARRAY_BUFFER, VBO);
//	glBufferSubData(GL_ARRAY_BUFFER, offset * sizeof(Vertex), amount * sizeof(Vertex), buffer->vertices.get_data() + offset);
//}

Mesh::~Mesh() {
	if(uploaded) {
		{
			std::lock_guard<std::mutex> lk(glContoller->meshDeleteMutex);
			glContoller->glDelete.push({VAO, VBO});
		}
	}
}

void Mesh::update() {
	//if(uploaded) {
	//	{
	//		std::lock_guard<std::mutex> lk(glContoller->meshUpdateMutex);
	//		glContoller->glUpdate.push(this);
	//	}
	//}
}

void Mesh::draw(unsigned int primitive) const {
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, verticesUpdated);
	glBindVertexArray(0);
}