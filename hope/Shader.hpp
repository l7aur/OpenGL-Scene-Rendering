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
private:
	GLuint shaderID;
	GLuint uniformProjection, uniformModel, uniformView;

	std::string readFile(const char* path);
	void compileShader(const char* vertexCode, const char* fragmentCode);
	void addShader(GLuint theProgram, const char* shaderCode, GLenum shaderType);
};