#include "Shader.hpp"

Shader::Shader()
{
}

void Shader::useShader()
{
	glUseProgram(shaderID);
}

void Shader::clearShader()
{
	if (shaderID != 0) {
		glDeleteProgram(shaderID);
		shaderID = 0;
	}
	uniformModel = uniformProjection = 0;
}

void Shader::createFromString(const char* vertexCode, const char* fragmentCode)
{
	compileShader(vertexCode, fragmentCode);
}

void Shader::createFromFile(const char* vertexLocation, const char* fragmentLocation)
{
	std::string vertexString = readFile(vertexLocation);
	std::string fragmentString = readFile(fragmentLocation);

	const char* vertexCode = vertexString.c_str();
	const char* fragmentCode = fragmentString.c_str();
	
	compileShader(vertexCode, fragmentCode);
}

std::string Shader::readFile(const char* path)
{
	std::string content;
	std::ifstream fin(path, std::ios::in);
	if (!fin.is_open()) {
		printf("Failed to read %s! File doesn't exist!\n", path);
		return "";
	}
	std::string line = "";
	while (!fin.eof()) {
		std::getline(fin, line);
		content.append(line + "\n");
	}
	fin.close();
	return content;
}

void Shader::compileShader(const char* vertexCode, const char* fragmentCode)
{
	shaderID = glCreateProgram();
	if (!shaderID) {
		printf("Error creating shader program!\n");
		return;
	}
	addShader(shaderID, vertexCode, GL_VERTEX_SHADER);
	addShader(shaderID, fragmentCode, GL_FRAGMENT_SHADER);

	GLint result = 0;
	GLchar eLog[1024]{ '\0' };

	glLinkProgram(shaderID);
	glGetProgramiv(shaderID, GL_LINK_STATUS, &result);
	if (!result) {
		glGetProgramInfoLog(shaderID, sizeof(eLog), NULL, eLog);
		printf("Error linking the shader program: %s!\n", eLog);
		return;
	}
	glValidateProgram(shaderID);
	glGetProgramiv(shaderID, GL_VALIDATE_STATUS, &result);
	if (!result) {
		glGetProgramInfoLog(shaderID, sizeof(eLog), NULL, eLog);
		printf("Error validating the shader program: %s!\n", eLog);
		return;
	}

	uniformModel = glGetUniformLocation(shaderID, "model");
	uniformProjection = glGetUniformLocation(shaderID, "projection");
	uniformView = glGetUniformLocation(shaderID, "view");
	uniformAmbientI = glGetUniformLocation(shaderID, "directionalLight.ambientIntensity");
	uniformAmbientColour = glGetUniformLocation(shaderID, "directionalLight.colour");
	uniformDiffuseI = glGetUniformLocation(shaderID, "directionalLight.diffuseIntensity");
	uniformDirection = glGetUniformLocation(shaderID, "directionalLight.direction");
	uniformEyePosition = glGetUniformLocation(shaderID, "eyePosition");
	uniformSpecularI = glGetUniformLocation(shaderID, "material.specularIntensity");
	uniformShininess = glGetUniformLocation(shaderID, "material.shininess");
}

void Shader::addShader(GLuint theProgram, const char* shaderCode, GLenum shaderType)
{
	GLuint theShader = glCreateShader(shaderType);

	const GLchar* theCode[1];
	theCode[0] = shaderCode;

	GLint codeLength[1];
	codeLength[0] = (GLint)strlen(shaderCode);

	glShaderSource(theShader, 1, theCode, codeLength);
	glCompileShader(theShader);

	GLint result = 0;
	GLchar eLog[1024]{ '\0' };

	glGetShaderiv(theShader, GL_COMPILE_STATUS, &result);
	if (!result) {
		glGetShaderInfoLog(theShader, sizeof(eLog), NULL, eLog);
		printf("Error linking the %d shader program: %s!\n", shaderType, eLog);
		return;
	}
	glAttachShader(theProgram, theShader);
}
