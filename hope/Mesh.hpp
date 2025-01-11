#pragma once

#include "GL/glew.h"

class Mesh {
public:
	Mesh();
	~Mesh() { clearMesh(); };
	void createMesh(GLfloat* vertices, unsigned int* indices, unsigned int numOfVertices, unsigned int numOfIndices);
	void renderMesh();
	void clearMesh();
private:
	GLuint VAO, VBO, IBO;
	GLsizei indexCount;
};