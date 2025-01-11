#define GLEW_STATIC
#define STB_IMAGE_IMPLEMENTATION

#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Mesh.hpp"
#include "Shader.hpp"
#include "Window.hpp"
#include "Camera.hpp"
#include "Texture.hpp"
#include "Light.hpp"
#include "Material.hpp"

Window mainWindow;
Camera camera;
Light mainLight;
Material shiny, dull;
std::vector<Mesh*> meshList;
std::vector<Shader*> shaderList;

GLfloat deltaTime{ 0.0f };
GLfloat lastTime{ 0.0f };

static const char* mainVertexShaderPath = "shaders/main.vert";
static const char* mainFragmentShaderPath = "shaders/main.frag";

Texture brickTex, dirtTex;

static void computeDeltaTime() {
	GLfloat currTime = (GLfloat)glfwGetTime();
	deltaTime = currTime - lastTime;
	lastTime = currTime;
}

static void calcAvgNormals(unsigned int* indices, unsigned int indiceCount, GLfloat* vertices, unsigned int verticeCount, 
	unsigned int vLength, unsigned int normalOffset) {
	for (size_t i = 0; i < indiceCount; i += 3) {
		unsigned int in0 = indices[i] * vLength;
		unsigned int in1 = indices[i + 1] * vLength;
		unsigned int in2 = indices[i + 2] * vLength;
		glm::vec3 v1(vertices[in1] - vertices[in0], vertices[in1 + 1] - vertices[in0 + 1], vertices[in1 + 2] - vertices[in0 + 2]);
		glm::vec3 v2(vertices[in2] - vertices[in0], vertices[in2 + 1] - vertices[in0 + 1], vertices[in2 + 2] - vertices[in0 + 2]);
		glm::vec3 normal = glm::cross(v1, v2);
		normal = glm::normalize(normal);
		in0 += normalOffset;in1 += normalOffset;in2 += normalOffset;
		vertices[in0] += normal.x; vertices[in0 + 1] += normal.y; vertices[in0 + 2] += normal.z;
		vertices[in1] += normal.x; vertices[in1 + 1] += normal.y; vertices[in1 + 2] += normal.z;
		vertices[in2] += normal.x; vertices[in2 + 1] += normal.y; vertices[in2 + 2] += normal.z;
	}
	for (size_t i = 0; i < verticeCount / vLength; i++) {
		unsigned int nOffset = i * vLength + normalOffset;
		glm::vec3 vec(vertices[nOffset], vertices[nOffset + 1], vertices[nOffset + 2]);
		vec = glm::normalize(vec);
		vertices[nOffset] = vec.x; vertices[nOffset + 1] = vec.y; vertices[nOffset + 2] = vec.z;
	}
}

static void createObjects() {
	GLfloat vertices[] = {
		-1.0f, -1.0f, -0.6f,		0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 1.0f,		0.5f, 0.0f, 0.0f, 0.0f, 0.0f,
		1.0f, -1.0f, -0.6f,		1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,		0.5f, 1.0f, 0.0f, 0.0f, 0.0f
	};
	unsigned int indices[] = {
		0, 3, 1,
		1, 3, 2,
		2, 3, 0,
		0, 1, 2
	};
	calcAvgNormals(indices, 12, vertices, 32, 8, 5);
	Mesh* obj1 = new Mesh();
	obj1->createMesh(vertices, indices, 32, 12);
	meshList.push_back(obj1);
	Mesh* obj2 = new Mesh();
	obj2->createMesh(vertices, indices, 32, 12);
	meshList.push_back(obj2);
}

static void createShaders() {
	Shader* shader = new Shader();
	shader->createFromFile(mainVertexShaderPath, mainFragmentShaderPath);
	shaderList.push_back(shader);
}

static void cleanup() {
	for (auto& i : meshList)
		delete i;
	for (auto& i : shaderList)
		delete i;
}

int main() {
	mainWindow = Window(1366, 768);
	if(mainWindow.initialise("GKS") < 0)
		return -1;

	createObjects();
	createShaders();

	camera = Camera(glm::vec3{ 0.0f, 0.0f, 0.0f }, glm::vec3{ 0.0f, 1.0f, 0.0f }, -90.0f, 0.0f, 5.0f, 0.1f);

	brickTex = Texture("models/textures/brick.png");
	brickTex.loadTexture();
	dirtTex = Texture("models/textures/dirt.png");
	dirtTex.loadTexture();

	mainLight = Light(
		1.0f, 1.0f, 1.0f, 
		0.2f, 
		2.0f, -1.0f, -2.0f, 
		0.3f);

	shiny = Material(1.0f, 32.0f);
	dull = Material(0.3f, 4.0f);

	GLuint uniformProjection{ 0 }, uniformModel{ 0 }, uniformView{ 0 };
	GLuint uniformAmbientI{ 0 }, uniformAmbientColour{ 0 };
	GLuint uniformDiffuseI{ 0 }, uniformDirection{ 0 };
	GLuint uniformEyePosition{ 0 }, uniformShininess{ 0 }, uniformSpecularI{ 0 };
	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / (GLfloat)mainWindow.getBufferHeight(), 0.1f, 100.0f);

	while (!mainWindow.shouldClose()) {
		computeDeltaTime();
		glfwPollEvents();

		camera.keyControl(mainWindow.getKeys(), deltaTime);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shaderList.at(0)->useShader();
		uniformModel = shaderList.at(0)->getModelLoc();
		uniformProjection = shaderList.at(0)->getProjectionLoc();
		uniformView = shaderList.at(0)->getViewLoc();
		uniformAmbientI = shaderList.at(0)->getAmbientILoc();
		uniformAmbientColour = shaderList.at(0)->getAmbientColourLoc();
		uniformDiffuseI = shaderList.at(0)->getDiffuseILoc();
		uniformDirection = shaderList.at(0)->getDirectionLoc();
		uniformEyePosition = shaderList.at(0)->getEyePositionLoc();
		uniformSpecularI = shaderList.at(0)->getSpecularILoc();
		uniformShininess = shaderList.at(0)->getShininessLoc();
		mainLight.useLight(uniformAmbientI, uniformAmbientColour, uniformDiffuseI, uniformDirection);

		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.computeViewMatrix()));
		glm::vec3 eyePosition = camera.getPosition();
		glUniform3f(uniformEyePosition, eyePosition.x, eyePosition.y, eyePosition.z);

		glm::mat4 model(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, -2.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		brickTex.useTexture();
		shiny.useMaterial(uniformSpecularI, uniformShininess);
		meshList.at(0)->renderMesh();

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 4.0f, -2.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		dirtTex.useTexture();
		dull.useMaterial(uniformSpecularI, uniformShininess);
		meshList.at(1)->renderMesh();

		glUseProgram(0);

		mainWindow.swapBuffers();
	}
	cleanup();
	return 0;
}