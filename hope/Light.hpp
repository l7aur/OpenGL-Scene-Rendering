#pragma once

#include "GL/glew.h"
#include "glm/glm.hpp"

class Light {
public:
	Light() {};
	Light(
		GLfloat red, GLfloat green, GLfloat blue, 
		GLfloat ambientI,
		GLfloat dirX, GLfloat dirY, GLfloat dirZ,
		GLfloat diffuseI);
	void useLight(
		GLuint ambientILocation, GLuint ambientColourLocation,
		GLuint diffuseILocation, GLuint directionLocation
	);
	~Light() {};
private:
	glm::vec3 colour{ 1.0f, 1.0f, 1.0f };
	GLfloat ambientIntensity{ 1.0f };
	
	glm::vec3 direction{ 0.0f, -1.0f, 0.0f };
	GLfloat diffuseIntensity{ 0.5f };
};