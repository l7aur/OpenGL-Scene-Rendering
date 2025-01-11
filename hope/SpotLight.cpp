#include "SpotLight.hpp"

SpotLight::SpotLight()
	: PointLight()
{
}

SpotLight::SpotLight(
	GLfloat red, GLfloat green, GLfloat blue, 
	GLfloat ambientI, GLfloat diffuseI, 
	GLfloat posX, GLfloat posY, GLfloat posZ, 
	GLfloat dirX, GLfloat dirY, GLfloat dirZ, 
	GLfloat con, GLfloat lin, GLfloat exp,
	GLfloat e)
	:
	PointLight(red, green, blue, ambientI, diffuseI, posX, posY, posZ, con, lin, exp),
	direction(glm::normalize(glm::vec3{ dirX, dirY, dirZ })),
	edge(e), 
	procEdge(cosf(glm::radians(e)))
{
}

void SpotLight::useLight(
	GLuint ambientILocation, GLuint ambientColourLocation, GLuint diffuseILocation, 
	GLuint positionLocation, GLuint directionLocation, 
	GLuint constantLocation, GLuint linearLocation, GLuint exponentLocation, 
	GLuint edgeLocation)
{
	PointLight::useLight(
		ambientILocation, ambientColourLocation,
		diffuseILocation, positionLocation,
		constantLocation, linearLocation, exponentLocation);
	glUniform1f(edgeLocation, procEdge);
	glUniform3f(directionLocation, direction.x, direction.y, direction.z);
}

void SpotLight::setFlash(glm::vec3 pos, glm::vec3 dir)
{
	position = pos;
	direction = dir;
}
