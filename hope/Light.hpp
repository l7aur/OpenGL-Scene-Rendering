#pragma once

#include "GL/glew.h"
#include "glm/glm.hpp"

class Light {
public:
	Light() {};
	Light(
		GLfloat red, GLfloat green, GLfloat blue,
		GLfloat ambientI, GLfloat diffuseI);
	void useLight(GLuint ambientILocation, GLuint ambientColourLocation, GLuint diffuseILocation);
	~Light() {};
protected:
	glm::vec3 colour{ 1.0f, 1.0f, 1.0f };
	GLfloat ambientIntensity{ 1.0f };
	
	GLfloat diffuseIntensity{ 0.5f };
};