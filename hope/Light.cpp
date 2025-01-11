#include "Light.hpp"

Light::Light(
	GLfloat red, GLfloat green, GLfloat blue,
	GLfloat ambientI, GLfloat diffuseI)
	: 
	colour(red, green, blue), ambientIntensity(ambientI), diffuseIntensity(diffuseI)
{
}

void Light::useLight(GLuint ambientILocation, GLuint ambientColourLocation, GLuint diffuseILocation)
{
	glUniform3f(ambientColourLocation, colour.r, colour.g, colour.b);
	glUniform1f(ambientILocation, ambientIntensity);
	glUniform1f(diffuseILocation, diffuseIntensity);
}
