#include "DirectionalLight.hpp"

DirectionalLight::DirectionalLight()
	: Light()
{
}

DirectionalLight::DirectionalLight(
	GLfloat red, GLfloat green, GLfloat blue,
	GLfloat ambientI, GLfloat diffuseI,
	GLfloat dirX, GLfloat dirY, GLfloat dirZ)
	:
	Light(red, green, blue, ambientI, diffuseI), direction(glm::normalize(glm::vec3{ dirX, dirY, dirZ }))
{
}

void DirectionalLight::useLight(
	GLuint ambientILocation, GLuint ambientColourLocation, 
	GLuint diffuseILocation, GLuint directionLocation)
{
	Light::useLight(ambientILocation, ambientColourLocation, diffuseILocation);
	glUniform3f(directionLocation, direction.x, direction.y, direction.z);
}
