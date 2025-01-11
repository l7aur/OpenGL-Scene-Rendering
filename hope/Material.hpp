#pragma once

#include "GL/glew.h"

class Material {
public:
	Material() {};
	~Material() {};
	Material(GLfloat specularI, GLfloat shine);
	void useMaterial(GLuint specularILocation, GLuint shininessLocation);
private:
	GLfloat specularIntensity{ 0.0f };
	GLfloat shininess{ 0.0f };
};