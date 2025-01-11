#define GLEW_STATIC

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

Window mainWindow;
Camera camera;
std::vector<Mesh*> meshList;
std::vector<Shader*> shaderList;

GLfloat deltaTime{ 0.0f };
GLfloat lastTime{ 0.0f };

static const char* mainVertexShaderPath = "shaders/main.vert";
static const char* mainFragmentShaderPath = "shaders/main.frag";

static void computeDeltaTime() {
	GLfloat currTime = glfwGetTime();
	deltaTime = currTime - lastTime;
	lastTime = currTime;
}

static void createObjects() {
	GLfloat vertices[] = {
		-1.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 1.0f,
		1.0f, -1.0f, 0.0f,
		0.0f, 1.0f, 0.0f
	};
	unsigned int indices[] = {
		0, 3, 1,
		1, 3, 2,
		2, 3, 0,
		0, 1, 2
	};
	Mesh* obj1 = new Mesh();
	obj1->createMesh(vertices, indices, 12, 12);
	meshList.push_back(obj1);
	Mesh* obj2 = new Mesh();
	obj2->createMesh(vertices, indices, 12, 12);
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
	mainWindow = Window(800, 600);
	if(mainWindow.initialise("GKS") < 0)
		return -1;

	createObjects();
	createShaders();

	camera = Camera(glm::vec3{ 0.0f, 0.0f, 0.0f }, glm::vec3{ 0.0f, 1.0f, 0.0f }, -90.0f, 0.0f, 5.0f, 0.1f);

	GLuint uniformProjection{ 0 }, uniformModel{ 0 }, uniformView{ 0 };
	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / (GLfloat)mainWindow.getBufferHeight(), 0.1f, 100.0f);

	while (!mainWindow.shouldClose()) {
		computeDeltaTime();
		glfwPollEvents();

		camera.keyControl(mainWindow.getKeys(), deltaTime);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shaderList[0]->useShader();
		uniformModel = shaderList[0]->getModelLoc();
		uniformProjection = shaderList[0]->getProjectionLoc();
		uniformView = shaderList[0]->getViewLoc();

		glm::mat4 model(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, -2.5f));
		model = glm::scale(model, glm::vec3(0.4f, 0.4f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.computeViewMatrix()));
		meshList[0]->renderMesh();

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 1.0f, -2.5f));
		model = glm::scale(model, glm::vec3(0.4f, 0.4f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[1]->renderMesh();

		glUseProgram(0);

		mainWindow.swapBuffers();
	}
	cleanup();
	return 0;
}