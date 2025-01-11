#pragma once

#include <stdio.h>
#include <string>
#include <fstream>

#include "GL/glew.h"

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
	GLuint getAmbientILoc() const { return uniformAmbientI; };
	GLuint getAmbientColourLoc() const { return uniformAmbientColour; };
	GLuint getDiffuseILoc() const { return uniformDiffuseI; };
	GLuint getDirectionLoc() const { return uniformDirection; };
	GLuint getEyePositionLoc() const { return uniformEyePosition; };
	GLuint getSpecularILoc() const { return uniformSpecularI; };
	GLuint getShininessLoc() const { return uniformShininess; };
private:
	GLuint shaderID{0};
	GLuint uniformProjection{ 0 }, uniformModel{ 0 }, uniformView{ 0 };
	GLuint uniformAmbientI{ 0 }, uniformAmbientColour{ 0 };
	GLuint uniformDiffuseI{ 0 }, uniformDirection{ 0 };
	GLuint uniformEyePosition{ 0 };
	GLuint uniformSpecularI{ 0 }, uniformShininess{ 0 };
	std::string readFile(const char* path);
	void compileShader(const char* vertexCode, const char* fragmentCode);
	void addShader(GLuint theProgram, const char* shaderCode, GLenum shaderType);
};