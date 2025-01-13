#include "PointLight.hpp"

PointLight::PointLight() 
	: Light()
{
	position = glm::vec3(0.0f, 0.0f, 0.0f);
	constant = 1.0f;
	linear = 0.0f;
	exponent = 0.0f;
}

PointLight::PointLight(
	GLuint shadowWidth, GLuint shadowHeight,
	GLfloat near, GLfloat far,
	GLfloat red, GLfloat green, GLfloat blue,
	GLfloat ambientI, GLfloat diffuseI,
	GLfloat posX, GLfloat posY, GLfloat posZ,
	GLfloat con, GLfloat lin, GLfloat exp)
	: 
	Light(shadowWidth, shadowHeight, red, green, blue, ambientI, diffuseI),
	position(posX, posY, posZ), constant(con), linear(lin), exponent(exp),
	nearPlane(near), farPlane(far)
{
	float aspect = (float)shadowWidth / (float)shadowHeight;
	lightProjection = glm::perspective(glm::radians(90.0f), aspect, near, far);

	delete shadowM;
	shadowM = new OmnidirectionalShadowMap();
	shadowM->init(shadowWidth, shadowHeight);
}

void PointLight::useLight(GLuint ambientIntensityLocation, GLuint ambientColourLocation,
	GLuint diffuseIntensityLocation, GLuint positionLocation,
	GLuint constantLocation, GLuint linearLocation, GLuint exponentLocation)
{
	Light::useLight(ambientIntensityLocation, ambientColourLocation, diffuseIntensityLocation);
	
	glUniform3f(positionLocation, position.x, position.y, position.z);
	glUniform1f(constantLocation, constant);
	glUniform1f(linearLocation, linear);
	glUniform1f(exponentLocation, exponent);
}

std::vector<glm::mat4> PointLight::computeLightTransform()
{
	return {
		lightProjection * glm::lookAt(position, position + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
		lightProjection * glm::lookAt(position, position + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
		lightProjection * glm::lookAt(position, position + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
		lightProjection * glm::lookAt(position, position + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
		lightProjection * glm::lookAt(position, position + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
		lightProjection * glm::lookAt(position, position + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))
	};
}