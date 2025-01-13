#pragma once

#include <vector>

#include "Light.hpp"
#include "OmnidirectionalShadowMap.hpp"

class PointLight : public Light {
public:
	PointLight();
	PointLight(
		GLuint shadowWidth, GLuint shadowHeight,
		GLfloat near, GLfloat far,
		GLfloat red, GLfloat green, GLfloat blue,
		GLfloat ambientI, GLfloat diffuseI,
		GLfloat posX, GLfloat posY, GLfloat posZ,
		GLfloat con, GLfloat lin, GLfloat exp);
	~PointLight() {};
	void useLight(
		GLuint ambientILocation, GLuint ambientColourLocation, GLuint diffuseILocation,
		GLuint positionLocation,
		GLuint constantLocation, GLuint linearLocation, GLuint exponentLocation);
	std::vector<glm::mat4> computeLightTransform();
	GLfloat getFarPlane() const { return farPlane; };
	glm::vec3 getPosition() const { return position; };
protected:
	glm::vec3 position{0.0f, 2.0f, 0.0f,};
	GLfloat constant{ 1.0f }, linear{ 0.0f }, exponent{ 0.0f };
	GLfloat farPlane{ 1000.0f }, nearPlane{ 0.1f };
};