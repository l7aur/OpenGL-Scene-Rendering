#include "Shader.hpp"

Shader::Shader()
{
	for (size_t i = 0; i < MAX_POINT_LIGHTS; i++) {
		uniformPointLights[i].ambientIntensity = 0;
		uniformPointLights[i].colour = 0;
		uniformPointLights[i].constant = 0;
		uniformPointLights[i].diffuseIntensity = 0;
		uniformPointLights[i].exponent = 0;
		uniformPointLights[i].linear = 0;
		uniformPointLights[i].position = 0;
	}
	for (size_t i = 0; i < MAX_SPOT_LIGHTS; i++) {
		uniformSpotLights[i].ambientIntensity = 0;
		uniformSpotLights[i].colour = 0;
		uniformSpotLights[i].constant = 0;
		uniformSpotLights[i].diffuseIntensity = 0;
		uniformSpotLights[i].direction = 0;
		uniformSpotLights[i].edge = 0;
		uniformSpotLights[i].exponent = 0;
		uniformSpotLights[i].linear = 0;
		uniformSpotLights[i].position = 0;
	}
	uniformDirectionalLight.ambientIntensity = 0;
	uniformDirectionalLight.colour = 0;
	uniformDirectionalLight.diffuseIntensity = 0;
	uniformDirectionalLight.direction = 0;
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

void Shader::createFromFiles(const char* vertexLocation, const char* fragmentLocation)
{
	std::string vertexString = readFile(vertexLocation);
	std::string fragmentString = readFile(fragmentLocation);

	const char* vertexCode = vertexString.c_str();
	const char* fragmentCode = fragmentString.c_str();
	
	compileShader(vertexCode, fragmentCode);
}

void Shader::setDirectionalLight(DirectionalLight* dLight)
{
	dLight->useLight(
		uniformDirectionalLight.ambientIntensity, uniformDirectionalLight.colour,
		uniformDirectionalLight.diffuseIntensity, uniformDirectionalLight.direction);
}

void Shader::setPointLights(PointLight* lights, unsigned int lightCount)
{
	if (lightCount > MAX_POINT_LIGHTS)
		lightCount = MAX_POINT_LIGHTS;
	glUniform1i(uniformPointLightCount, lightCount);

	for (size_t i = 0; i < lightCount; i++) {
		lights[i].useLight(
			uniformPointLights[i].ambientIntensity, uniformPointLights[i].colour, uniformPointLights[i].diffuseIntensity, 
			uniformPointLights[i].position,
			uniformPointLights[i].constant, uniformPointLights[i].linear, uniformPointLights[i].exponent);
	}
}

void Shader::setSpotLights(SpotLight* lights, unsigned int spotCount)
{
	if (spotCount > MAX_SPOT_LIGHTS)
		spotCount = MAX_SPOT_LIGHTS;
	glUniform1i(uniformSpotLightCount, spotCount);

	for (size_t i = 0; i < spotCount; i++) {
		lights[i].useLight(
			uniformSpotLights[i].ambientIntensity, uniformSpotLights[i].colour, uniformSpotLights[i].diffuseIntensity,
			uniformSpotLights[i].position, uniformSpotLights[i].direction,
			uniformSpotLights[i].constant, uniformSpotLights[i].linear, uniformSpotLights[i].exponent,
			uniformSpotLights[i].edge);
	}
}

void Shader::setTexture(GLuint textureUnit) { glUniform1i(uniformTexture, textureUnit); }

void Shader::setDirectionalShadowMap(GLuint textureUnit) { glUniform1i(uniformDirectionalShadowM, textureUnit); }

void Shader::setDirectionalLightTransform(glm::mat4* lightT) { glUniformMatrix4fv(uniformDirectionalLightTransform, 1, GL_FALSE, glm::value_ptr(*lightT)); }

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
	uniformDirectionalLight.ambientIntensity = glGetUniformLocation(shaderID, "directionalLight.base.ambientIntensity");
	uniformDirectionalLight.colour = glGetUniformLocation(shaderID, "directionalLight.base.colour");
	uniformDirectionalLight.diffuseIntensity = glGetUniformLocation(shaderID, "directionalLight.base.diffuseIntensity");
	uniformDirectionalLight.direction = glGetUniformLocation(shaderID, "directionalLight.base.direction");
	uniformEyePosition = glGetUniformLocation(shaderID, "eyePosition");
	uniformSpecularI = glGetUniformLocation(shaderID, "material.specularIntensity");
	uniformShininess = glGetUniformLocation(shaderID, "material.shininess");

	uniformPointLightCount = glGetUniformLocation(shaderID, "pointLightCount");
	for (int i = 0; i < MAX_POINT_LIGHTS; i++) {
		char buffer[100] = { '\0'};
		snprintf(buffer, sizeof(buffer), "pointLights[%i].base.colour", i);
		uniformPointLights[i].colour = glGetUniformLocation(shaderID, buffer);

		snprintf(buffer, sizeof(buffer), "pointLights[%i].base.ambientIntensity", i);
		uniformPointLights[i].ambientIntensity = glGetUniformLocation(shaderID, buffer);

		snprintf(buffer, sizeof(buffer), "pointLights[%i].base.diffuseIntensity", i);
		uniformPointLights[i].diffuseIntensity = glGetUniformLocation(shaderID, buffer);

		snprintf(buffer, sizeof(buffer), "pointLights[%i].position", i);
		uniformPointLights[i].position = glGetUniformLocation(shaderID, buffer);

		snprintf(buffer, sizeof(buffer), "pointLights[%i].constant", i);
		uniformPointLights[i].constant = glGetUniformLocation(shaderID, buffer);

		snprintf(buffer, sizeof(buffer), "pointLights[%i].linear", i);
		uniformPointLights[i].linear = glGetUniformLocation(shaderID, buffer);

		snprintf(buffer, sizeof(buffer), "pointLights[%i].exponent", i);
		uniformPointLights[i].exponent = glGetUniformLocation(shaderID, buffer);
	}

	uniformSpotLightCount = glGetUniformLocation(shaderID, "spotLightCount");
	for (int i = 0; i < MAX_SPOT_LIGHTS; i++) {
		char buffer[100] = { '\0' };
		snprintf(buffer, sizeof(buffer), "spotLights[%i].base.base.colour", i);
		uniformSpotLights[i].colour = glGetUniformLocation(shaderID, buffer);

		snprintf(buffer, sizeof(buffer), "spotLights[%i].base.base.ambientIntensity", i);
		uniformSpotLights[i].ambientIntensity = glGetUniformLocation(shaderID, buffer);

		snprintf(buffer, sizeof(buffer), "spotLights[%i].base.base.diffuseIntensity", i);
		uniformSpotLights[i].diffuseIntensity = glGetUniformLocation(shaderID, buffer);

		snprintf(buffer, sizeof(buffer), "spotLights[%i].base.position", i);
		uniformSpotLights[i].position = glGetUniformLocation(shaderID, buffer);

		snprintf(buffer, sizeof(buffer), "spotLights[%i].base.constant", i);
		uniformSpotLights[i].constant = glGetUniformLocation(shaderID, buffer);

		snprintf(buffer, sizeof(buffer), "spotLights[%i].base.linear", i);
		uniformSpotLights[i].linear = glGetUniformLocation(shaderID, buffer);

		snprintf(buffer, sizeof(buffer), "spotLights[%i].base.exponent", i);
		uniformSpotLights[i].exponent = glGetUniformLocation(shaderID, buffer);

		snprintf(buffer, sizeof(buffer), "spotLights[%i].direction", i);
		uniformSpotLights[i].direction = glGetUniformLocation(shaderID, buffer);

		snprintf(buffer, sizeof(buffer), "spotLights[%i].edge", i);
		uniformSpotLights[i].edge = glGetUniformLocation(shaderID, buffer);
	}

	uniformTexture = glGetUniformLocation(shaderID, "theTexture");
	uniformDirectionalLightTransform = glGetUniformLocation(shaderID, "directionalLightTransform");
	uniformDirectionalShadowM = glGetUniformLocation(shaderID, "directionalShadowMap");
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
