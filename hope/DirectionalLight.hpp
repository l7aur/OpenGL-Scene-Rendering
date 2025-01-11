#pragma once

#include "Light.hpp"

class DirectionalLight: public Light {
public:
	DirectionalLight();
	DirectionalLight(
		GLfloat red, GLfloat green, GLfloat blue, 
		GLfloat ambientI, GLfloat diffuseI,
		GLfloat dirX, GLfloat dirY, GLfloat dirZ);
	void useLight(
		GLuint ambientILocation, GLuint ambientColourLocation,
		GLuint diffuseILocation, GLuint directionLocation);
	~DirectionalLight() {};
private:
	glm::vec3 direction{ 0.0f, -1.0f, 0.0f };
};