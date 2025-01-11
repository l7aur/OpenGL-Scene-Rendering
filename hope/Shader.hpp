#pragma once

#include <stdio.h>
#include <string>
#include <fstream>

#include "GL/glew.h"

#include "DirectionalLight.hpp"
#include "PointLight.hpp"
#include "SpotLight.hpp"
#include "CommonValues.hpp"

class Shader {
public:
	Shader();
	~Shader() { clearShader(); };
	void useShader();
	void clearShader();
	void createFromString(const char *vertexCode, const char* fragmentCode);
	void createFromFile(const char *vertexLocation, const char* fragmentLocation);
	GLuint getProjectionLoc() const { return uniformProjection; };
	GLuint getModelLoc() const { return uniformModel; };
	GLuint getViewLoc() const { return uniformView; };
	GLuint getEyePositionLoc() const { return uniformEyePosition; };
	GLuint getSpecularILoc() const { return uniformSpecularI; };
	GLuint getShininessLoc() const { return uniformShininess; };

	void setDirectionalLight(DirectionalLight* dLight);
	void setPointLights(PointLight* lights, unsigned int lightCount);
	void setSpotLights(SpotLight* lights, unsigned int spotCount);

private:
	int pointLightCount{ 0 };
	int spotLightCount{ 0 };
	GLuint shaderID{0};
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

	std::string readFile(const char* path);
	void compileShader(const char* vertexCode, const char* fragmentCode);
	void addShader(GLuint theProgram, const char* shaderCode, GLenum shaderType);
};