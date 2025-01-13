#include "Light.hpp"

Light::Light(
	GLuint shadowWidth, GLuint shadowHeight,
	GLfloat red, GLfloat green, GLfloat blue,
	GLfloat ambientI, GLfloat diffuseI)
	: 
	colour(red, green, blue), 
	ambientIntensity(ambientI), 
	diffuseIntensity(diffuseI)
{
	shadowM = new ShadowMap();
	shadowM->init(shadowWidth, shadowHeight);
}

void Light::useLight(GLuint ambientILocation, GLuint ambientColourLocation, GLuint diffuseILocation)
{
	if (isOn) {
		glUniform3f(ambientColourLocation, colour.r, colour.g, colour.b);
		glUniform1f(ambientILocation, ambientIntensity);
		glUniform1f(diffuseILocation, diffuseIntensity);
		return;
	}
	glUniform1f(ambientILocation, 0.0f);
	glUniform1f(diffuseILocation, 0.0f);
}
