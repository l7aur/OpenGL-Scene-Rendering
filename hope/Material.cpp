#include "Material.hpp"

Material::Material(GLfloat specularI, GLfloat shine)
	: specularIntensity(specularI), shininess(shine)
{
}

void Material::useMaterial(GLuint specularILocation, GLuint shininessLocation)
{
	glUniform1f(specularILocation, specularIntensity);
	glUniform1f(shininessLocation, shininess);
}
