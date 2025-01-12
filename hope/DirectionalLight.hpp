#pragma once

#include "Light.hpp"

class DirectionalLight: public Light {
public:
	DirectionalLight();
	DirectionalLight(
		GLuint shadowWidth, GLuint shadowHeight,
		GLfloat red, GLfloat green, GLfloat blue, 
		GLfloat ambientI, GLfloat diffuseI,
		GLfloat dirX, GLfloat dirY, GLfloat dirZ);
	~DirectionalLight() {};
	void useLight(
		GLuint ambientILocation, GLuint ambientColourLocation,
		GLuint diffuseILocation, GLuint directionLocation);
	glm::mat4 computeLightTransform();
private:
	glm::vec3 direction{ 0.0f, -1.0f, 0.0f };
};