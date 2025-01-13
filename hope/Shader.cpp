#include "Shader.hpp"

Shader::Shader() {
	for (size_t i = 0; i < MAX_POINT_LIGHTS; i++) {
		uniformPointLights[i].ambientIntensity = uniformPointLights[i].colour = 0;
		uniformPointLights[i].constant = uniformPointLights[i].diffuseIntensity = 0;
		uniformPointLights[i].exponent = uniformPointLights[i].linear = uniformPointLights[i].position = 0;
	}
	for (size_t i = 0; i < MAX_SPOT_LIGHTS; i++) {
		uniformSpotLights[i].ambientIntensity = uniformSpotLights[i].colour = 0;
		uniformSpotLights[i].constant = uniformSpotLights[i].diffuseIntensity = 0;
		uniformSpotLights[i].direction = uniformSpotLights[i].edge = 0;
		uniformSpotLights[i].exponent = uniformSpotLights[i].linear = uniformSpotLights[i].position = 0;
	}
	uniformDirectionalLight.ambientIntensity = uniformDirectionalLight.colour = 0;
	uniformDirectionalLight.diffuseIntensity = uniformDirectionalLight.direction = 0;
	for (size_t i = 0; i < MAX_POINT_LIGHTS + MAX_SPOT_LIGHTS; i++)
		uniformOmniShadowM[i].shadowMap = uniformOmniShadowM[i].farPlane = 0;
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

void Shader::createFromString(const char* vertexCode, const char* geometryCode, const char* fragmentCode)
{
	compileShader(vertexCode, geometryCode, fragmentCode);
}

void Shader::createFromFiles(const char* vertexLocation, const char* fragmentLocation)
{
	std::string vertexString = readFile(vertexLocation);
	std::string fragmentString = readFile(fragmentLocation);

	const char* vertexCode = vertexString.c_str();
	const char* fragmentCode = fragmentString.c_str();
	
	compileShader(vertexCode, fragmentCode);
}

void Shader::createFromFiles(const char* vertexLocation, const char* geometryLocation, const char* fragmentLocation)
{
	std::string vertexString = readFile(vertexLocation);
	std::string geometryString = readFile(geometryLocation);
	std::string fragmentString = readFile(fragmentLocation);

	const char* vertexCode = vertexString.c_str();
	const char* geometryCode = geometryString.c_str();
	const char* fragmentCode = fragmentString.c_str();

	compileShader(vertexCode, geometryCode, fragmentCode);
}

void Shader::validate()
{
	GLint result = 0;
	GLchar eLog[1024]{ '\0' };
	glValidateProgram(shaderID);
	glGetProgramiv(shaderID, GL_VALIDATE_STATUS, &result);
	if (!result) {
		glGetProgramInfoLog(shaderID, sizeof(eLog), NULL, eLog);
		printf("Error validating the shader program: %s!\n", eLog);
		return;
	}
}

void Shader::setDirectionalLight(DirectionalLight* dLight)
{
	dLight->useLight(
		uniformDirectionalLight.ambientIntensity, uniformDirectionalLight.colour,
		uniformDirectionalLight.diffuseIntensity, uniformDirectionalLight.direction);
}

void Shader::setPointLights(PointLight* lights, unsigned int lightCount, unsigned int textureUnit, unsigned int offset)
{
	if (lightCount > MAX_POINT_LIGHTS)
		lightCount = MAX_POINT_LIGHTS;
	glUniform1i(uniformPointLightCount, lightCount);

	for (size_t i = 0; i < lightCount; i++) {
		lights[i].useLight(
			uniformPointLights[i].ambientIntensity, uniformPointLights[i].colour, uniformPointLights[i].diffuseIntensity, 
			uniformPointLights[i].position,
			uniformPointLights[i].constant, uniformPointLights[i].linear, uniformPointLights[i].exponent);

		lights[i].getShadowMap()->read(GL_TEXTURE0 + textureUnit + i);
		glUniform1i(uniformOmniShadowM[i + offset].shadowMap, textureUnit + i);

		float f = lights[i].getFarPlane();
		glUniform1f(uniformOmniShadowM[i + offset].farPlane, f);
	}
}

void Shader::setSpotLights(SpotLight* lights, unsigned int spotCount, unsigned int textureUnit, unsigned int offset)
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

		lights[i].getShadowMap()->read(GL_TEXTURE0 + textureUnit + i);
		glUniform1i(uniformOmniShadowM[i + offset].shadowMap, textureUnit + i);

		float f = lights[i].getFarPlane();
		glUniform1f(uniformOmniShadowM[i + offset].farPlane, f);
	}
}

void Shader::setOmniLightMatrices(std::vector<glm::mat4> lightMatrices)
{
	for (size_t i = 0; i < 6; i++)
		glUniformMatrix4fv(uniformLightMatrices[i], 1, GL_FALSE, glm::value_ptr(lightMatrices.at(i)));
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
	
	compileProgram();
}

void Shader::compileShader(const char* vertexCode, const char* geometryCode, const char* fragmentCode)
{
	shaderID = glCreateProgram();
	if (!shaderID) {
		printf("Error creating shader program!\n");
		return;
	}
	addShader(shaderID, vertexCode, GL_VERTEX_SHADER);
	addShader(shaderID, geometryCode, GL_GEOMETRY_SHADER);
	addShader(shaderID, fragmentCode, GL_FRAGMENT_SHADER);

	compileProgram();
}

void Shader::compileProgram()
{
	GLint result = 0;
	GLchar eLog[1024]{ '\0' };

	glLinkProgram(shaderID);
	glGetProgramiv(shaderID, GL_LINK_STATUS, &result);
	if (!result) {
		glGetProgramInfoLog(shaderID, sizeof(eLog), NULL, eLog);
		printf("Error linking the shader program: %s!\n", eLog);
		return;
	}

	uniformModel = glGetUniformLocation(shaderID, "model");
	uniformProjection = glGetUniformLocation(shaderID, "projection");
	uniformView = glGetUniformLocation(shaderID, "view");
	uniformDirectionalLight.ambientIntensity = glGetUniformLocation(shaderID, "directionalLight.base.ambientIntensity");
	uniformDirectionalLight.colour = glGetUniformLocation(shaderID, "directionalLight.base.colour");
	uniformDirectionalLight.diffuseIntensity = glGetUniformLocation(shaderID, "directionalLight.base.diffuseIntensity");
	uniformDirectionalLight.direction = glGetUniformLocation(shaderID, "directionalLight.direction");
	uniformEyePosition = glGetUniformLocation(shaderID, "eyePosition");
	uniformSpecularI = glGetUniformLocation(shaderID, "material.specularIntensity");
	uniformShininess = glGetUniformLocation(shaderID, "material.shininess");

	uniformPointLightCount = glGetUniformLocation(shaderID, "pointLightCount");
	for (size_t i = 0; i < MAX_POINT_LIGHTS; i++) {
		char buffer[100] = { '\0' };
		snprintf(buffer, sizeof(buffer), "pointLights[%zu].base.colour", i);
		uniformPointLights[i].colour = glGetUniformLocation(shaderID, buffer);

		snprintf(buffer, sizeof(buffer), "pointLights[%zu].base.ambientIntensity", i);
		uniformPointLights[i].ambientIntensity = glGetUniformLocation(shaderID, buffer);

		snprintf(buffer, sizeof(buffer), "pointLights[%zu].base.diffuseIntensity", i);
		uniformPointLights[i].diffuseIntensity = glGetUniformLocation(shaderID, buffer);

		snprintf(buffer, sizeof(buffer), "pointLights[%zu].position", i);
		uniformPointLights[i].position = glGetUniformLocation(shaderID, buffer);

		snprintf(buffer, sizeof(buffer), "pointLights[%zu].constant", i);
		uniformPointLights[i].constant = glGetUniformLocation(shaderID, buffer);

		snprintf(buffer, sizeof(buffer), "pointLights[%zu].linear", i);
		uniformPointLights[i].linear = glGetUniformLocation(shaderID, buffer);

		snprintf(buffer, sizeof(buffer), "pointLights[%zu].exponent", i);
		uniformPointLights[i].exponent = glGetUniformLocation(shaderID, buffer);
	}

	uniformSpotLightCount = glGetUniformLocation(shaderID, "spotLightCount");
	for (size_t i = 0; i < MAX_SPOT_LIGHTS; i++) {
		char buffer[100] = { '\0' };

		snprintf(buffer, sizeof(buffer), "spotLights[%zu].base.base.colour", i);
		uniformSpotLights[i].colour = glGetUniformLocation(shaderID, buffer);

		snprintf(buffer, sizeof(buffer), "spotLights[%zu].base.base.ambientIntensity", i);
		uniformSpotLights[i].ambientIntensity = glGetUniformLocation(shaderID, buffer);

		snprintf(buffer, sizeof(buffer), "spotLights[%zu].base.base.diffuseIntensity", i);
		uniformSpotLights[i].diffuseIntensity = glGetUniformLocation(shaderID, buffer);

		snprintf(buffer, sizeof(buffer), "spotLights[%zu].base.position", i);
		uniformSpotLights[i].position = glGetUniformLocation(shaderID, buffer);

		snprintf(buffer, sizeof(buffer), "spotLights[%zu].base.constant", i);
		uniformSpotLights[i].constant = glGetUniformLocation(shaderID, buffer);

		snprintf(buffer, sizeof(buffer), "spotLights[%zu].base.linear", i);
		uniformSpotLights[i].linear = glGetUniformLocation(shaderID, buffer);

		snprintf(buffer, sizeof(buffer), "spotLights[%zu].base.exponent", i);
		uniformSpotLights[i].exponent = glGetUniformLocation(shaderID, buffer);

		snprintf(buffer, sizeof(buffer), "spotLights[%zu].direction", i);
		uniformSpotLights[i].direction = glGetUniformLocation(shaderID, buffer);

		snprintf(buffer, sizeof(buffer), "spotLights[%zu].edge", i);
		uniformSpotLights[i].edge = glGetUniformLocation(shaderID, buffer);
	}

	uniformTexture = glGetUniformLocation(shaderID, "theTexture");
	uniformDirectionalLightTransform = glGetUniformLocation(shaderID, "directionalLightTransform");
	uniformDirectionalShadowM = glGetUniformLocation(shaderID, "directionalShadowMap");

	uniformOmniLightPos = glGetUniformLocation(shaderID, "lightPos");
	uniformFarPlane = glGetUniformLocation(shaderID, "farPlane");

	for (size_t i = 0; i < 6; i++) {
		char buffer[100] = { '\0' };
		snprintf(buffer, sizeof(buffer), "lightMatrices[%zu]", i);
		uniformLightMatrices[i] = glGetUniformLocation(shaderID, buffer);
	}

	for (size_t i = 0; i < MAX_POINT_LIGHTS + MAX_SPOT_LIGHTS; i++) {
		char buffer[100]{ '\0' };
		snprintf(buffer, sizeof(buffer), "omniShadowMaps[%zu].shadowMap", i);
		uniformOmniShadowM[i].shadowMap = glGetUniformLocation(shaderID, buffer);

		snprintf(buffer, sizeof(buffer), "omniShadowMaps[%zu].farPlane", i);
		uniformOmniShadowM[i].farPlane = glGetUniformLocation(shaderID, buffer);
	}
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
		printf("Error compiling the %d shader: %s!\n", shaderType, eLog);
		return;
	}
	glAttachShader(theProgram, theShader);
}