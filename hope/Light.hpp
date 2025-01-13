#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "ShadowMap.hpp"

class Light {
public:
	Light() {};
	Light(
		GLuint shadowWidth, GLuint shadowHeight,
		GLfloat red, GLfloat green, GLfloat blue,
		GLfloat ambientI, GLfloat diffuseI);
	void useLight(GLuint ambientILocation, GLuint ambientColourLocation, GLuint diffuseILocation);
	void toggle() { isOn = !isOn; };
	ShadowMap* getShadowMap() const { return shadowM; };
	~Light() {};
protected:
	bool isOn{ true };

	glm::vec3 colour{ 1.0f, 1.0f, 1.0f };
	GLfloat ambientIntensity{ 1.0f };

	GLfloat diffuseIntensity{ 0.5f };

	glm::mat4 lightProjection{ 1.0f };
	ShadowMap* shadowM{ nullptr };
};