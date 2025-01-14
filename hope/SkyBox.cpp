#include "SkyBox.hpp"

SkyBox::SkyBox(std::vector<std::string> facePaths)
{
	skyShader = new Shader();
	skyShader->createFromFiles("shaders/skybox.vert", "shaders/skybox.frag");

	uniformView = skyShader->getViewLoc();
	uniformProjection = skyShader->getProjectionLoc();

	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);

	int width{ 0 }, height{ 0 }, bitDepth{ 0 };
	for (size_t i = 0; i < 6; i++) {
		unsigned char* texData = stbi_load(facePaths.at(i).c_str(), &width, &height, &bitDepth, 0);
		if (!texData) {
			printf("Failed to find %s!\n", facePaths.at(i).c_str());
			return;
		}
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, texData);
		stbi_image_free(texData);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	unsigned int skyBoxIndices[]{
		0, 1, 2,
		2, 1, 3,
		2, 3, 5,
		5, 3, 7,
		5, 7, 4,
		4, 7, 6,
		4, 6, 0,
		0, 6, 1,
		4, 0, 5,
		5, 0, 2,
		1, 6, 3,
		3, 6, 7
	};
	float skyBoxVertices[]{
		-1.0f, 1.0f, -1.0f,			0.0f, 0.0f,			0.0f, 0.0f, 0.0f,
		-1.0f, -1.0f, -1.0f,		0.0f, 0.0f,			0.0f, 0.0f, 0.0f,
		1.0f, 1.0f, -1.0f,			0.0f, 0.0f,			0.0f, 0.0f, 0.0f,
		1.0f, -1.0f, -1.0f,			0.0f, 0.0f,			0.0f, 0.0f, 0.0f,
		-1.0f, 1.0f, 1.0f,			0.0f, 0.0f,			0.0f, 0.0f, 0.0f,
		1.0f, 1.0f, 1.0f,			0.0f, 0.0f,			0.0f, 0.0f, 0.0f,
		-1.0f, -1.0f, 1.0f,			0.0f, 0.0f,			0.0f, 0.0f, 0.0f,
		1.0f, -1.0f, 1.0f,			0.0f, 0.0f,			0.0f, 0.0f, 0.0f
	};

	skyMesh = new Mesh();
	skyMesh->createMesh(skyBoxVertices, skyBoxIndices, 64, 36);
}

SkyBox::~SkyBox()
{
	//DE CE NU??
	//if (skyShader)
		//delete skyShader;
	//if (skyMesh)
		//delete skyMesh;
	//skyShader = nullptr;
	//skyMesh = nullptr;
}

void SkyBox::draw(glm::mat4 viewM, glm::mat4 projectionM)
{
	viewM = glm::mat4(glm::mat3(viewM)); //remove translations

	glDepthMask(GL_FALSE);

	skyShader->useShader();

	glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projectionM));
	glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(viewM));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);

	skyShader->validate();

	skyMesh->renderMesh();

	glDepthMask(GL_TRUE);
}
