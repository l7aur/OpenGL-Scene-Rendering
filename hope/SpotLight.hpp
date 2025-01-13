#pragma once

#include "PointLight.hpp"

class SpotLight : public PointLight {
public:
	SpotLight();
	SpotLight(
		GLuint shadowWidth, GLuint shadowHeight,
		GLfloat near, GLfloat far,
		GLfloat red, GLfloat green, GLfloat blue,
		GLfloat ambientI, GLfloat diffuseI,
		GLfloat posX, GLfloat posY, GLfloat posZ,
		GLfloat dirX, GLfloat dirY, GLfloat dirZ,
		GLfloat con, GLfloat lin, GLfloat exp,
		GLfloat e);
	~SpotLight() {};
	void useLight(
		GLuint ambientILocation, GLuint ambientColourLocation, GLuint diffuseILocation,
		GLuint positionLocation, GLuint directionLocation,
		GLuint constantLocation, GLuint linearLocation, GLuint exponentLocation,
		GLuint edgeLocation);
	void setFlash(glm::vec3 pos, glm::vec3 dir);
private:
	glm::vec3 direction{ 0.0, -1.0f, 0.0f };
	GLfloat edge{ 0.0f }, procEdge{ 1.0f };
};