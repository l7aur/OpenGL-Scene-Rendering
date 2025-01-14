#pragma once

#include "CommonValues.hpp"

#include <vector>
#include <string>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.hpp"
#include "Mesh.hpp"

class SkyBox {
public:
	SkyBox() {};
	SkyBox(std::vector<std::string> facePaths);
	~SkyBox();
	void draw(glm::mat4 viewM, glm::mat4 projectionM);
private:
	Mesh* skyMesh{ nullptr };
	Shader* skyShader{ nullptr };
	GLuint textureId{ 0 };
	GLuint uniformProjection{ 0 }, uniformView{ 0 };
};