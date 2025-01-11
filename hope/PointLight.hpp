#pragma once

#include "Light.hpp"

class PointLight : public Light {
public:
	PointLight();
	PointLight(
		GLfloat red, GLfloat green, GLfloat blue,
		GLfloat ambientI, GLfloat diffuseI,
		GLfloat posX, GLfloat posY, GLfloat posZ,
		GLfloat con, GLfloat lin, GLfloat exp);
	~PointLight() {};
	void useLight(
		GLuint ambientILocation, GLuint ambientColourLocation, GLuint diffuseILocation,
		GLuint positionLocation,
		GLuint constantLocation, GLuint linearLocation, GLuint exponentLocation);
protected:
	glm::vec3 position{0.0f, 2.0f, 0.0f,};
	GLfloat constant{ 1.0f }, linear{ 0.0f }, exponent{ 0.0f };
};