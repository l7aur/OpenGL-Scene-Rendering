#include "Light.hpp"

Light::Light(
	GLfloat red, GLfloat green, GLfloat blue, 
	GLfloat ambientI,
	GLfloat dirX, GLfloat dirY, GLfloat dirZ,
	GLfloat diffuseI)
	: 
	colour(red, green, blue), ambientIntensity(ambientI), direction(dirX, dirY, dirZ), diffuseIntensity(diffuseI)
{
}

void Light::useLight(
	GLuint ambientILocation, GLuint ambientColourLocation,
	GLuint diffuseILocation, GLuint directionLocation)
{
	glUniform3f(ambientColourLocation, colour.r, colour.g, colour.b);
	glUniform1f(ambientILocation, ambientIntensity);

	glUniform3f(directionLocation, direction.x, direction.y, direction.z);
	glUniform1f(diffuseILocation, diffuseIntensity);
}
