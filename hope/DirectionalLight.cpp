#include "DirectionalLight.hpp"

DirectionalLight::DirectionalLight()
	: Light()
{
	lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, 0.1f, 100.0f);
}

DirectionalLight::DirectionalLight(
	GLuint shadowWidth, GLuint shadowHeight,
	GLfloat red, GLfloat green, GLfloat blue,
	GLfloat ambientI, GLfloat diffuseI,
	GLfloat dirX, GLfloat dirY, GLfloat dirZ)
	:
	Light(shadowWidth, shadowHeight, red, green, blue, ambientI, diffuseI), 
	direction(glm::vec3{ dirX, dirY, dirZ })
{
	lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, 0.1f, 100.0f);
}

void DirectionalLight::useLight(
	GLuint ambientILocation, GLuint ambientColourLocation, 
	GLuint diffuseILocation, GLuint directionLocation)
{
	Light::useLight(ambientILocation, ambientColourLocation, diffuseILocation);
	glUniform3f(directionLocation, direction.x, direction.y, direction.z);
}

glm::mat4 DirectionalLight::computeLightTransform()
{
	// if looking straight up or down, the up vector must change into {1, 0, 0} or {0, 0, 1}
	return lightProjection * glm::lookAt(-direction, glm::vec3{ 0.0f, 0.0f, 0.0f }, glm::vec3{ 0.0f, 1.0f, 0.0f });
}
