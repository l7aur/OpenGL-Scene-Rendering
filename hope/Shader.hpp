#pragma once

#include <stdio.h>
#include <string>
#include <fstream>
#include <iostream>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "CommonValues.hpp"
#include "DirectionalLight.hpp"
#include "PointLight.hpp"
#include "SpotLight.hpp"

class Shader {
public:
	Shader();
	~Shader() { clearShader(); };
	void useShader();
	void clearShader();
	void createFromString(const char* vertexCode, const char* fragmentCode);
	void createFromString(const char* vertexCode, const char* geometryCode, const char* fragmentCode);
	void createFromFiles(const char* vertexLocation, const char* fragmentLocation);
	void createFromFiles(const char* vertexLocation, const char* geometryLocation, const char* fragmentLocation);
	void validate();

	GLuint getProjectionLoc() const { return uniformProjection; };
	GLuint getModelLoc() const { return uniformModel; };
	GLuint getViewLoc() const { return uniformView; };
	GLuint getEyePositionLoc() const { return uniformEyePosition; };
	GLuint getSpecularILoc() const { return uniformSpecularI; };
	GLuint getShininessLoc() const { return uniformShininess; };
	GLuint getOmniLightPosLoc() const { return uniformOmniLightPos; };
	GLuint getFarPlanePos() const { return uniformFarPlane; };

	void setDirectionalLight(DirectionalLight* dLight);
	void setPointLights(PointLight* lights, unsigned int lightCount, unsigned int textureUnit = 0, unsigned int offset = 0);
	void setSpotLights(SpotLight* lights, unsigned int spotCount, unsigned int textureUnit, unsigned int offset);
	void setTexture(GLuint textureUnit) { glUniform1i(uniformTexture, textureUnit); };
	void setDirectionalShadowMap(GLuint textureUnit) { glUniform1i(uniformDirectionalShadowM, textureUnit); };
	void setDirectionalLightTransform(glm::mat4* lightT) { glUniformMatrix4fv(uniformDirectionalLightTransform, 1, GL_FALSE, glm::value_ptr(*lightT)); };
	void setOmniLightMatrices(std::vector<glm::mat4> lightMatrices);
private:
	int pointLightCount{ 0 };
	int spotLightCount{ 0 };
	GLuint shaderID{ 0 };
	GLuint uniformProjection{ 0 }, uniformModel{ 0 }, uniformView{ 0 };
	GLuint uniformEyePosition{ 0 };
	GLuint uniformSpecularI{ 0 }, uniformShininess{ 0 };
	GLuint uniformPointLightCount{ 0 };
	GLuint uniformSpotLightCount{ 0 };
	struct {
		GLuint colour;
		GLuint ambientIntensity;
		GLuint diffuseIntensity;

		GLuint direction;
	} uniformDirectionalLight;
	struct {
		GLuint colour;
		GLuint ambientIntensity;
		GLuint diffuseIntensity;

		GLuint position;
		GLuint constant, linear, exponent;
	} uniformPointLights[MAX_POINT_LIGHTS];
	struct {
		GLuint colour;
		GLuint ambientIntensity;
		GLuint diffuseIntensity;

		GLuint position;
		GLuint constant, linear, exponent;
		GLuint direction, edge;
	} uniformSpotLights[MAX_SPOT_LIGHTS];
	GLuint uniformDirectionalLightTransform{ 0 }, uniformDirectionalShadowM{ 0 };
	GLuint uniformTexture{ 0 };
	GLuint uniformOmniLightPos{ 0 }, uniformFarPlane{ 0 };
	GLuint uniformLightMatrices[6]{ 0, 0, 0, 0, 0, 0 };
	struct {
		GLuint shadowMap;
		GLuint farPlane;
	} uniformOmniShadowM[MAX_POINT_LIGHTS + MAX_SPOT_LIGHTS];
	std::string readFile(const char* path);
	void compileShader(const char* vertexCode, const char* fragmentCode);
	void compileShader(const char* vertexCode, const char* geometryCode, const char* fragmentCode);
	void compileProgram();
	void addShader(GLuint theProgram, const char* shaderCode, GLenum shaderType);
};