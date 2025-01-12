#include "PointLight.hpp"

PointLight::PointLight()
	: Light()
{
}

PointLight::PointLight(
	GLfloat red, GLfloat green, GLfloat blue, 
	GLfloat ambientI, GLfloat diffuseI, 
	GLfloat posX, GLfloat posY, GLfloat posZ, 
	GLfloat con, GLfloat lin, GLfloat exp)
	:
	Light(1024, 1024, red, green, blue, ambientI, diffuseI), 
	position(posX, posY, posZ), constant(con), linear(lin), exponent(exp)
{
}

void PointLight::useLight(
	GLuint ambientILocation, GLuint ambientColourLocation, GLuint diffuseILocation, 
	GLuint positionLocation, 
	GLuint constantLocation, GLuint linearLocation, GLuint exponentLocation)
{
	Light::useLight(ambientILocation, ambientColourLocation, diffuseILocation);
	glUniform3f(positionLocation, position.x, position.y, position.z);
	glUniform1f(constantLocation, constant);
	glUniform1f(linearLocation, linear);
	glUniform1f(exponentLocation, exponent);
}
